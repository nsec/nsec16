from an esp32 folder where 'nsec-ctf-addon' firmware is ready to be built with `add_definitions(-DCTF_ADDON_ADMIN_MODE)` commented out in top CMakelists.txt

```idf.py build

parttool.py write_partition --partition-name=ota_1 --input build\nsec-ctf-addon.bin

```

remove comment to define `add_definitions(-DCTF_ADDON_ADMIN_MODE)` in top CMakelists.txt

```idf.py build flash monitor
crypto 999
# continue if no FAIL
storage 111
raw_toggle
storage 222
```

Handy otatool commands (they trigger reboot):
```
# switch to ota_1
otatool.py switch_ota_partition --slot 1

# switch to firmware (doesn't really erase the ota data, it lies)
otatool.py erase_otadata

# really delete the ota data
parttool.py erase_partition --partition-name=ota_1
```

QA production:
validate that the following command doesn't work:
```
idf.py monitor
storage 4
``

