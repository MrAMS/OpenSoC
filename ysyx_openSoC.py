#!/usr/bin/env python3

#
# This file is part of LiteX-Boards.
#
# Copyright (c) 2021 Lucas Teske <lucas@teske.com.br>
# SPDX-License-Identifier: BSD-2-Clause

from migen import *

from litex.gen import *

from litex.build.io import DDROutput

import myplatform

from litex.soc.cores.clock import *
from litex.soc.integration.soc_core import *
from litex.soc.integration.builder import *
from litex.soc.cores.video import VideoHDMIPHY
from litex.soc.cores.led import LedChaser

from litex.soc.interconnect.csr import *

from litedram.modules import IS42S16160
from litedram.phy import GENSDRPHY, HalfRateGENSDRPHY

from liteeth.phy.ecp5rgmii import LiteEthPHYRGMII

from litex.soc.cores.timer import *
from litex.soc.cores.gpio import *
from litex.soc.cores.bitbang import I2CMaster
from litex.soc.cores.spi import SPIMaster

# CRG ----------------------------------------------------------------------------------------------

class _CRG(LiteXModule):
    def __init__(self, platform, sys_clk_freq, use_internal_osc=False, with_video_pll=False, sdram_rate="1:1"):
        self.rst    = Signal()
        self.cd_sys = ClockDomain()
        if sdram_rate == "1:2":
            self.cd_sys2x    = ClockDomain()
            self.cd_sys2x_ps = ClockDomain()
        else:
            self.cd_sys_ps = ClockDomain()

        self.cd_eth = ClockDomain()
        # # #

        # Clk / Rst
        if not use_internal_osc:
            clk = platform.request("clk25")
            clk_freq = 25e6
        else:
            clk = Signal()
            div = 5
            self.specials += Instance("OSCG",
                p_DIV = div,
                o_OSC = clk
            )
            clk_freq = 310e6/div

        rst_n = platform.request("cpu_reset_n")

        # PLL
        self.pll = pll = ECP5PLL()
        self.comb += pll.reset.eq(~rst_n | self.rst)
        pll.register_clkin(clk, clk_freq)
        pll.create_clkout(self.cd_sys,    sys_clk_freq)
        if sdram_rate == "1:2":
            pll.create_clkout(self.cd_sys2x,    2*sys_clk_freq)
            pll.create_clkout(self.cd_sys2x_ps, 2*sys_clk_freq, phase=180) # Idealy 90° but needs to be increased.
        else:
           pll.create_clkout(self.cd_sys_ps, sys_clk_freq, phase=180) # Idealy 90° but needs to be increased.
        pll.create_clkout(self.cd_eth, 50e6)

        # Video PLL
        if with_video_pll:
            self.video_pll = video_pll = ECP5PLL()
            self.comb += video_pll.reset.eq(~rst_n | self.rst)
            video_pll.register_clkin(clk, clk_freq)
            self.cd_hdmi   = ClockDomain()
            self.cd_hdmi5x = ClockDomain()
            video_pll.create_clkout(self.cd_hdmi,    25e6, margin=0)
            video_pll.create_clkout(self.cd_hdmi5x, 125e6, margin=0)

        # SDRAM clock
        sdram_clk = ClockSignal("sys2x_ps" if sdram_rate == "1:2" else "sys_ps")
        self.specials += DDROutput(1, 0, platform.request("sdram_clock"), sdram_clk)

# BaseSoC ------------------------------------------------------------------------------------------

class BaseSoC(SoCCore):
    def __init__(self, sys_clk_freq=50e6, toolchain="trellis",
        with_led_chaser        = True,
        with_spi_flash         = False,
        use_internal_osc       = False,
        sdram_rate             = "1:1",
        with_video_terminal    = False,
        with_video_framebuffer = False,
        with_ethernet          = False,
        with_etherbone         = False,
        eth_ip                 = "192.168.1.50",
        eth_dynamic_ip         = False,
        **kwargs):
        platform = myplatform.Platform(toolchain=toolchain)

        # CRG --------------------------------------------------------------------------------------
        with_video_pll = with_video_terminal or with_video_framebuffer
        self.crg = _CRG(platform, sys_clk_freq, use_internal_osc=use_internal_osc, with_video_pll=with_video_pll, sdram_rate=sdram_rate)

        # SoCCore ----------------------------------------------------------------------------------
        SoCCore.__init__(self, platform, int(sys_clk_freq), ident="OpenSoC on Muselab iCESugar Pro", **kwargs)

        # Leds -------------------------------------------------------------------------------------
        if with_led_chaser:
            ledn = platform.request_all("user_led_n")
            self.leds = LedChaser(pads=ledn, sys_clk_freq=sys_clk_freq)

        # SPI Flash --------------------------------------------------------------------------------
        if with_spi_flash:
            from litespi.modules import W25Q256
            from litespi.opcodes import SpiNorFlashOpCodes as Codes
            self.add_spi_flash(mode="1x", module=W25Q256(Codes.READ_1_1_1))

        # # SDR SDRAM --------------------------------------------------------------------------------
        # if not self.integrated_main_ram_size:
        #     sdrphy_cls = HalfRateGENSDRPHY if sdram_rate == "1:2" else GENSDRPHY
        #     self.sdrphy = sdrphy_cls(platform.request("sdram"))
        #     self.add_sdram("sdram",
        #         phy           = self.sdrphy,
        #         module        = IS42S16160(sys_clk_freq, sdram_rate),
        #         l2_cache_size = kwargs.get("l2_size", 8192)
        #     )
        from litespi.spi_nor_flash_module import SpiNorFlashModule
        from litespi.opcodes import SpiNorFlashOpCodes as Codes
        class PSRAM(SpiNorFlashModule):
            # ref https://gitlab.informatik.uni-bremen.de/fbrning/esp-idf/-/blob/6313ea00881f7d779c56e72085ca36218bb43cfd/components/esp32/spiram_psram.c
            manufacturer_id =   0x000d          # FIXME ESP-PSRAM64H
            device_id       =   0x9F
            name            =   "ESP-PSRAM64H"
            total_size      =   64*1024*1024    # bytes  # 64Mbit
            page_size       =   1024            # bytes  # 1KB页大小
            total_pages     =   total_size/page_size
            supported_opcodes = [
                Codes.READ_1_1_1,
                Codes.READ_1_1_1_FAST,
                Codes.READ_1_4_4,
                Codes.PP_1_1_1,
                Codes.PP_1_4_4,
            ]
            dummy_bits = 0x5 # FIXME 未验证正确性 参考：#define PSRAM_FAST_READ_QUAD_DUMMY 0x5
        
        self.add_spi_ram(name="psram", mode="4x", clk_freq=30e6, module=PSRAM(Codes.READ_1_4_4))
        from litex.soc.integration.soc import SoCRegion
        self.bus.add_region("main_ram", SoCRegion(
            origin = self.bus.regions["psram"].origin,
            size = self.bus.regions["psram"].size,
            linker = True
        ))

        # Video ------------------------------------------------------------------------------------
        if with_video_terminal or with_video_framebuffer:
            self.videophy = VideoHDMIPHY(platform.request("gpdi"), clock_domain="hdmi")
            if with_video_terminal:
                self.add_video_terminal(phy=self.videophy, timings="640x480@60Hz", clock_domain="hdmi")
            if with_video_framebuffer:
                self.add_video_framebuffer(phy=self.videophy, timings="640x480@60Hz", clock_domain="hdmi")

        # Ethernet / Etherbone ---------------------------------------------------------------------
        if with_ethernet or with_etherbone:
            from liteeth.phy.rmii import LiteEthPHYRMII
            self.ethphy = LiteEthPHYRMII(
                clock_pads = self.platform.request("eth_clocks"),
                pads = self.platform.request("eth"))
            if with_ethernet:
                self.add_ethernet(phy=self.ethphy, dynamic_ip=eth_dynamic_ip)
            if with_etherbone:
                self.add_etherbone(phy=self.ethphy, ip_address=eth_ip)

        # SPI0
        self.spi0 = SPIMaster(pads=platform.request("spi0"), data_width=32, sys_clk_freq=sys_clk_freq, spi_clk_freq=10e6)
        
        # I2C0
        self.i2c0 = I2CMaster(pads=platform.request("i2c0"))

        # Timer
        self.timer1 = Timer()

        # GPIO
        self.gpio0 = GPIOTristate(pads=platform.request("gpio0"), with_irq=True)

        # Interrupts
        self.irq.add("gpio0", use_loc_if_exists=True)
        self.irq.add("timer1",  use_loc_if_exists=True)

# Build --------------------------------------------------------------------------------------------

def main():
    from litex.build.parser import LiteXArgumentParser
    parser = LiteXArgumentParser(platform=myplatform.Platform, description="OpenSoC")
    parser.add_target_argument("--sys-clk-freq", default=50e6, help="System clock frequency.")
    sdopts = parser.target_group.add_mutually_exclusive_group()
    sdopts.add_argument("--with-spi-sdcard",         action="store_true",  help="Enable SPI-mode SDCard support.")
    sdopts.add_argument("--with-sdcard",             action="store_true",  help="Enable SDCard support.")
    parser.add_target_argument("--with-spi-flash",   action="store_true",  help="Enable SPI Flash (MMAPed).")
    parser.add_target_argument("--use-internal-osc", action="store_true",  help="Use internal oscillator.")
    parser.add_target_argument("--sdram-rate",       default="1:1",        help="SDRAM Rate (1:1 Full Rate or 1:2 Half Rate).")
    viopts = parser.target_group.add_mutually_exclusive_group()
    viopts.add_argument("--with-video-terminal",    action="store_true", help="Enable Video Terminal (HDMI).")
    viopts.add_argument("--with-video-framebuffer", action="store_true", help="Enable Video Framebuffer (HDMI).")
    ethopts = parser.target_group.add_mutually_exclusive_group()
    ethopts.add_argument("--with-ethernet",         action="store_true",    help="Add Ethernet.")
    ethopts.add_argument("--with-etherbone",        action="store_true",    help="Add EtherBone.")
    parser.add_target_argument("--eth-ip",          default="192.168.1.50", help="Etherbone IP address.")
    parser.add_target_argument("--eth-dynamic-ip",  action="store_true",    help="Enable dynamic Ethernet IP addresses setting.")

    args = parser.parse_args()

    soc = BaseSoC(
        sys_clk_freq           = args.sys_clk_freq,
        toolchain              = args.toolchain,
        use_internal_osc       = args.use_internal_osc,
        sdram_rate             = args.sdram_rate,
        with_spi_flash         = args.with_spi_flash,
        with_video_terminal    = args.with_video_terminal,
        with_video_framebuffer = args.with_video_framebuffer,
        with_ethernet          = args.with_ethernet,
        with_etherbone         = args.with_etherbone,
        eth_ip                 = args.eth_ip,
        eth_dynamic_ip         = args.eth_dynamic_ip,
        **parser.soc_argdict
    )
    if args.with_spi_sdcard:
        soc.add_spi_sdcard()
    if args.with_sdcard:
        soc.add_sdcard()

    builder = Builder(soc, **parser.builder_argdict)
    if args.build:
        builder.build(**parser.toolchain_argdict)

    if args.load:
        prog = soc.platform.create_programmer()
        prog.load_bitstream(builder.get_bitstream_filename(mode="sram"))

if __name__ == "__main__":
    main()
