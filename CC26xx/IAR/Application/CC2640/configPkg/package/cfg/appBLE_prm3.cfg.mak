# invoke SourceDir generated makefile for appBLE.prm3
appBLE.prm3: .libraries,appBLE.prm3
.libraries,appBLE.prm3: package/cfg/appBLE_prm3.xdl
	$(MAKE) -f C:\ti\simplelink\ble_cc26xx_2_01_01_44627_for_EW7.42\Projects\ble\SimpleBLEPeripheral\CC26xx\IAR\Config/src/makefile.libs

clean::
	$(MAKE) -f C:\ti\simplelink\ble_cc26xx_2_01_01_44627_for_EW7.42\Projects\ble\SimpleBLEPeripheral\CC26xx\IAR\Config/src/makefile.libs clean

