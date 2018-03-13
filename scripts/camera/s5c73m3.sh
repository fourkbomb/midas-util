#!/bin/sh
 
# Links set up
# Disconnect links
media-ctl -r

# s5c73m3 at fimc.0.capture
media-ctl --links '"s5p-mipi-csis.0":1 -> "FIMC.0":0 [1]'

# Rear facing camera

media-ctl --set-v4l2 "\"S5C73M3\":0 [fmt: VYUY8_2X8/640x480]"
#media-ctl --set-v4l2 "\"S5C73M3\":1 [fmt: 0x5001/2560x1920]"

media-ctl --set-v4l2 "\"S5C73M3-OIF\":0 [fmt: VYUY8_2X8/640x480]"
#media-ctl --set-v4l2 "\"S5C73M3-OIF\":1 [fmt: 0x5001/2560x1920]"
media-ctl --set-v4l2 "\"S5C73M3-OIF\":2 [fmt: VYUY8_2X8/640x480]"

# set pixel format on s5p-mipi-csis.0 pads
media-ctl --set-v4l2 "\"s5p-mipi-csis.0\":0 [fmt: VYUY8_2X8/640x480]"
media-ctl --set-v4l2 "\"s5p-mipi-csis.0\":1 [fmt: VYUY8_2X8/640x480]"

# set pixel format on FIMC.0 pads
media-ctl --set-v4l2 "\"FIMC.0\":0 [fmt: VYUY8_2X8/640x480]"
media-ctl --set-v4l2 "\"FIMC.0\":2 [fmt: VYUY8_2X8/640x480]"

echo Format at entity FIMC.0, pad 0:
media-ctl --get-v4l2 '"FIMC.0":0'
echo Format at entity FIMC.0, pad 2:
media-ctl --get-v4l2 '"FIMC.0":2'

#echo 1 > /sys/module/s5c73m3/parameters/boot_from_rom
#cat /sys/module/s5c73m3/parameters/boot_from_rom
