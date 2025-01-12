#
# This file is part of LiteX-Boards.
#
# Copyright (c) 2021 Lucas Teske <lucas@teske.com.br>
# SPDX-License-Identifier: BSD-2-Clause

# The Muselab IceSugar Pro PCB and IOs have been documented by @wuxx
# https://github.com/wuxx/icesugar-pro

from litex.build.generic_platform import *
from litex.build.lattice import LatticeECP5Platform
from litex.build.lattice.programmer import EcpDapProgrammer

# IOs ----------------------------------------------------------------------------------------------

_io = [
    # Clk
    ("clk25", 0, Pins("P6"), IOStandard("LVCMOS33")),

    # Led
    ("user_led_n", 0, Pins("B11"), IOStandard("LVCMOS33")), # Red
    ("user_led_n", 1, Pins("A11"), IOStandard("LVCMOS33")), # Green
    ("user_led_n", 2, Pins("A12"), IOStandard("LVCMOS33")), # Blue

    ("rgb_led", 0,
        Subsignal("r", Pins("B11")),
        Subsignal("g", Pins("A11")),
        Subsignal("b", Pins("A12")),
        IOStandard("LVCMOS33"),
    ),

    # Reset button
    ("cpu_reset_n", 0, Pins("L14"), IOStandard("LVCMOS33"), Misc("PULLMODE=UP")),

    # Serial
    ("serial", 0, # iCELink
        Subsignal("tx", Pins("B9")),
        Subsignal("rx", Pins("A9")),
        IOStandard("LVCMOS33")
    ),

    # SPIFlash (W25Q256JV (32MB))
    ("spiflash", 0,
        Subsignal("cs_n", Pins("N8")),
        # https://github.com/m-labs/nmigen-boards/pull/38
        #Subsignal("clk",  Pins("")), driven through USRMCLK
        Subsignal("mosi", Pins("T8")),
        Subsignal("miso", Pins("T7")),
        IOStandard("LVCMOS33"),
    ),

    # SDRAM (IS42S16160B (32MB))
    ("sdram_clock", 0, Pins("R15"), IOStandard("LVCMOS33")),
    ("sdram", 0,
        Subsignal("a", Pins(
            "H15 B13 B12 J16 J15 R12 K16 R13",
            "T13 K15 A13 R14 T14")),
        Subsignal("dq", Pins(
            "F16 E15 F15 D14 E16 C15 D16 B15",
            "R16 P16 P15 N16 N14 M16 M15 L15")),
        Subsignal("we_n",  Pins("A15")),
        Subsignal("ras_n", Pins("B16")),
        Subsignal("cas_n", Pins("G16")),
        Subsignal("cs_n", Pins("A14")),
        Subsignal("cke",  Pins("L16")),
        Subsignal("ba",    Pins("G15 B14")),
        Subsignal("dm",   Pins("C16 T15")),
        IOStandard("LVCMOS33"),
        Misc("SLEWRATE=FAST")
    ),

    # SDCard
    ("spisdcard", 0,
        Subsignal("clk",  Pins("J12")),
        Subsignal("mosi", Pins("H12"), Misc("PULLMODE=UP")),
        Subsignal("cs_n", Pins("G12"), Misc("PULLMODE=UP")),
        Subsignal("miso", Pins("K12"), Misc("PULLMODE=UP")),
        Misc("SLEWRATE=FAST"),
        IOStandard("LVCMOS33"),
    ),
    ("sdcard", 0,
        Subsignal("clk", Pins("J12")),
        Subsignal("cmd", Pins("H12"), Misc("PULLMODE=UP")),
        Subsignal("data", Pins("K12 L12 F12 G12"), Misc("PULLMODE=UP")),
        Misc("SLEWRATE=FAST"),
        IOStandard("LVCMOS33")
     ),

    # GPDI
    ("gpdi", 0,
        Subsignal("clk_p",   Pins("E2"), IOStandard("LVCMOS33"), Misc("DRIVE=4")),
        Subsignal("clk_n",   Pins("D3"), IOStandard("LVCMOS33"), Misc("DRIVE=4")),
        Subsignal("data0_p", Pins("G1"), IOStandard("LVCMOS33"), Misc("DRIVE=4")),
        Subsignal("data0_n", Pins("F1"), IOStandard("LVCMOS33"), Misc("DRIVE=4")),
        Subsignal("data1_p", Pins("J1"), IOStandard("LVCMOS33"), Misc("DRIVE=4")),
        Subsignal("data1_n", Pins("H2"), IOStandard("LVCMOS33"), Misc("DRIVE=4")),
        Subsignal("data2_p", Pins("L1"), IOStandard("LVCMOS33"), Misc("DRIVE=4")),
        Subsignal("data2_n", Pins("K2"), IOStandard("LVCMOS33"), Misc("DRIVE=4")),
    ),

    # RMII Ethernet PHY (WaveShare Board)
    # Assumed to be modified to be PMOD-compatible (TX1 tied to MDIO)
    # Position is P4 header "top half" (toward the GPDI connector)
    ("eth_clocks", 0,
        Subsignal("ref_clk", Pins("D5")),
        IOStandard("LVCMOS33"),
    ),
    ("eth", 0,
        Subsignal("rx_data", Pins("D4 C3")),
        Subsignal("crs_dv",  Pins("C4")),
        Subsignal("tx_en",   Pins("E4")),
        Subsignal("tx_data", Pins("E3 R7")),
        IOStandard("LVCMOS33"),
    ),

    ("spi0", 0,
        Subsignal("cs_n", Pins("T6")),
        Subsignal("mosi", Pins("R5")),
        Subsignal("miso", Pins("P7")),
        Subsignal("clk",  Pins("R3")),
        # Subsignal("io0",  Pins("R7")),
        # Subsignal("io1",  Pins("R6")),
        # Subsignal("io2",  Pins("T4")),
        # Subsignal("io3",  Pins("T3")),
        IOStandard("LVCMOS33"),
    ),

    # pins map follow Pmod Interface Type 6A (expanded I²C)
    ("i2c0", 0,
        Subsignal("sda", Pins("A7")),
        Subsignal("scl", Pins("A8")),
        IOStandard("LVCMOS33"),
    ),

    # pins map follow Pmod Interface Type 1A (Expanded GPIO)
    ("gpio0", 0,
        Subsignal("io0", Pins("N3")),
        Subsignal("io1", Pins("M2")),
        Subsignal("io2", Pins("L2")),
        Subsignal("io3", Pins("G2")),
        Subsignal("io4", Pins("P1")),
        Subsignal("io5", Pins("N1")),
        Subsignal("io6", Pins("M1")),
        Subsignal("io7", Pins("K1")),
        # spi0 pmod gpio
        Subsignal("io8",  Pins("R4")), # for LCD_DC
        Subsignal("io9",  Pins("R6")),
        Subsignal("io10",  Pins("T4")),
        Subsignal("io11",  Pins("T3")),
        # i2c0 pmod gpio
        Subsignal("io12",  Pins("A5")),
        Subsignal("io13",  Pins("A6")),
        Subsignal("io14",  Pins("B5")),
        Subsignal("io15",  Pins("B6")),
        Subsignal("io16",  Pins("B7")),
        Subsignal("io17",  Pins("B8")),

        IOStandard("LVCMOS33"),
    ),

    # pmod pins order: cs_n dq0 dq1 clk dq2 dq3
    ("psram4x", 0,
        Subsignal("cs_n", Pins("E1")),
        Subsignal("dq",  Pins("C2", "B2", "F2", "D1")),
        Subsignal("clk",  Pins("A2")),
        IOStandard("LVCMOS33"),
    ),


]

# from colorlight_i5.py adapted to icesugar pro
# https://github.com/wuxx/icesugar-pro/blob/master/doc/iCESugar-pro-pinmap.png
_connectors = [
    # ("pmode", "N3  M2  L2  G2  P1  N1  M1  K1"),
    # ("pmodf", "T6  R5  R4  R3  P7  R6  T4  T3"),
]

# Platform -----------------------------------------------------------------------------------------

class Platform(LatticeECP5Platform):
    default_clk_name   = "clk25"
    default_clk_period = 1e9/25e6

    def __init__(self, toolchain="trellis"):
        device     = "LFE5U-25F-6BG256C"
        io         = _io
        connectors = _connectors
        LatticeECP5Platform.__init__(self, device, io, connectors=connectors, toolchain=toolchain)

    def create_programmer(self):
        return EcpDapProgrammer()

    def do_finalize(self, fragment):
        LatticeECP5Platform.do_finalize(self, fragment)
        self.add_period_constraint(self.lookup_request("clk25", loose=True), 1e9/25e6)
