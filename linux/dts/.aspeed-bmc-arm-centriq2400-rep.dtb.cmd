cmd_arch/arm/boot/dts/aspeed-bmc-arm-centriq2400-rep.dtb := mkdir -p arch/arm/boot/dts/ ; gcc -E -Wp,-MD,arch/arm/boot/dts/.aspeed-bmc-arm-centriq2400-rep.dtb.d.pre.tmp -nostdinc -I./scripts/dtc/include-prefixes -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.aspeed-bmc-arm-centriq2400-rep.dtb.dts.tmp arch/arm/boot/dts/aspeed-bmc-arm-centriq2400-rep.dts ; ./scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/aspeed-bmc-arm-centriq2400-rep.dtb -b 0 -iarch/arm/boot/dts/ -i./scripts/dtc/include-prefixes -Wno-unit_address_vs_reg -Wno-unit_address_format -Wno-avoid_unnecessary_addr_size -Wno-alias_paths -Wno-graph_child_address -Wno-simple_bus_reg -Wno-unique_unit_address -Wno-pci_device_reg  -d arch/arm/boot/dts/.aspeed-bmc-arm-centriq2400-rep.dtb.d.dtc.tmp arch/arm/boot/dts/.aspeed-bmc-arm-centriq2400-rep.dtb.dts.tmp ; cat arch/arm/boot/dts/.aspeed-bmc-arm-centriq2400-rep.dtb.d.pre.tmp arch/arm/boot/dts/.aspeed-bmc-arm-centriq2400-rep.dtb.d.dtc.tmp > arch/arm/boot/dts/.aspeed-bmc-arm-centriq2400-rep.dtb.d

source_arch/arm/boot/dts/aspeed-bmc-arm-centriq2400-rep.dtb := arch/arm/boot/dts/aspeed-bmc-arm-centriq2400-rep.dts

deps_arch/arm/boot/dts/aspeed-bmc-arm-centriq2400-rep.dtb := \
  arch/arm/boot/dts/aspeed-g5.dtsi \
  scripts/dtc/include-prefixes/dt-bindings/clock/aspeed-clock.h \
  scripts/dtc/include-prefixes/dt-bindings/gpio/aspeed-gpio.h \
  scripts/dtc/include-prefixes/dt-bindings/gpio/gpio.h \
  arch/arm/boot/dts/openbmc-flash-layout.dtsi \

arch/arm/boot/dts/aspeed-bmc-arm-centriq2400-rep.dtb: $(deps_arch/arm/boot/dts/aspeed-bmc-arm-centriq2400-rep.dtb)

$(deps_arch/arm/boot/dts/aspeed-bmc-arm-centriq2400-rep.dtb):
