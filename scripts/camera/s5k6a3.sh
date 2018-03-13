#!/bin/sh
# Links set up
echo 1
media-ctl --links '"s5p-mipi-csis.1":1 -> "FIMC-LITE.1":0 [1]'
echo 2
media-ctl --links '"FIMC-LITE.1":2 -> "FIMC-IS-ISP":0 [1]'
echo 3
media-ctl --links '"FIMC-IS-ISP":1 -> "FIMC.1":1 [1]'
#media-ctl --links '"FIMC-IS-ISP":1 -> "FIMC.1":0 [1]'

# s5c73m3 at fimc-lite.0.capture
echo 4
#media-ctl --links '"s5p-mipi-csis.1":1 -> "FIMC-LITE.0":0 [1]'
echo 5
#media-ctl --links '"FIMC-LITE.0":1 -> "fimc-lite.0.capture":0 [1]'
echo 6
#media-ctl --links '"FIMC-IS-ISP":2 -> "fimc-is-isp.capture":0 [1]'

# FICM-IS CAC margin 16x12

# set format at S5K6A3 sensor
media-ctl --set-v4l2 '"S5K6A3 16-0010":0 [fmt: SGRBG10/1296x732]'

# set pixel format on s5p-mipi-csis.1 pads
media-ctl --set-v4l2 '"s5p-mipi-csis.1":0 [fmt: SGRBG10/1296x732]'
media-ctl --set-v4l2 '"s5p-mipi-csis.1":1 [fmt: SGRBG10/1296x732]'

# set pixel format on FIMC-LITE.1 pads
media-ctl --set-v4l2 '"FIMC-LITE.1":0 [fmt: SGRBG10/1296x732]'
media-ctl --set-v4l2 '"FIMC-LITE.1":1 [fmt: SGRBG10/1296x732]'

# Set pixel format on FIMC-IS-ISP subdev pads
media-ctl --set-v4l2 '"FIMC-IS-ISP":0 [fmt: YUYV2X8/1296x732]'
media-ctl --set-v4l2 '"FIMC-IS-ISP":1 [fmt: YUYV2X8/1280x720]'

# Set pixel format on FIMC-IS-ISP subdev pads
media-ctl --set-v4l2 '"FIMC-IS-ISP":0 [fmt: YUYV2X8/1296x732]'
media-ctl --set-v4l2 '"FIMC-IS-ISP":1 [fmt: YUYV2X8/1280x720]'

# Set pixel format on FIMC.1 subdev pads
media-ctl --set-v4l2 '"FIMC.1":0 [fmt: YUYV2X8/1280x720]'
media-ctl --set-v4l2 '"FIMC.1":1 [fmt: YUYV2X8/1280x720]'
media-ctl --set-v4l2 '"FIMC.1":2 [fmt: YUYV2X8/1280x720]'


echo Format at entity s5p-mipi-csis.1, pad 0:
media-ctl --get-v4l2 '"s5p-mipi-csis.1":0'
echo Format at entity s5p-mipi-csis.1, pad 1:
media-ctl --get-v4l2 '"s5p-mipi-csis.1":1'

echo Format at entity FIMC-LITE.1, pad 0:
media-ctl --get-v4l2 '"FIMC-LITE.1":0'
echo Format at entity FIMC-LITE.1, pad 1:
media-ctl --get-v4l2 '"FIMC-LITE.1":1'

echo Format at entity FIMC-IS-ISP, pad 0:
media-ctl --get-v4l2 '"FIMC-IS-ISP":0'
echo Format at entity FIMC-IS-ISP, pad 1:
media-ctl --get-v4l2 '"FIMC-IS-ISP":1'

echo Format at entity FIMC.1, pad 0:
media-ctl --get-v4l2 '"FIMC.1":0'
echo Format at entity FIMC.1, pad 1:
media-ctl --get-v4l2 '"FIMC.1":1'
