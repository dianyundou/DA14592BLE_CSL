#!/bin/bash

#########################################################################################
# Copyright (C) 2015-2019 Renesas Electronics Corporation and/or its affiliates.
# All rights reserved. Confidential Information.
#
# This software ("Software") is supplied by Renesas Electronics Corporation and/or its
# affiliates ("Renesas"). Renesas grants you a personal, non-exclusive, non-transferable,
# revocable, non-sub-licensable right and license to use the Software, solely if used in
# or together with Renesas products. You may make copies of this Software, provided this
# copyright notice and disclaimer ("Notice") is included in all such copies. Renesas
# reserves the right to change or discontinue the Software at any time without notice.
#
# THE SOFTWARE IS PROVIDED "AS IS". RENESAS DISCLAIMS ALL WARRANTIES OF ANY KIND,
# WHETHER EXPRESS, IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. TO THE
# MAXIMUM EXTENT PERMITTED UNDER LAW, IN NO EVENT SHALL RENESAS BE LIABLE FOR ANY DIRECT,
# INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE, EVEN IF RENESAS HAS BEEN ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGES. USE OF THIS SOFTWARE MAY BE SUBJECT TO TERMS AND CONDITIONS CONTAINED IN
# AN ADDITIONAL AGREEMENT BETWEEN YOU AND RENESAS. IN CASE OF CONFLICT BETWEEN THE TERMS
# OF THIS NOTICE AND ANY SUCH ADDITIONAL LICENSE AGREEMENT, THE TERMS OF THE AGREEMENT
# SHALL TAKE PRECEDENCE. BY CONTINUING TO USE THIS SOFTWARE, YOU AGREE TO THE TERMS OF
# THIS NOTICE.IF YOU DO NOT AGREE TO THESE TERMS, YOU ARE NOT PERMITTED TO USE THIS
# SOFTWARE.
#########################################################################################

echo
echo
echo "Welcome to Dialog suota image builder!"
echo
echo

if [[ $# -lt 1 ]]; then
	echo "usage suota_build_img.sh <file1> <file2>"
	echo
fi


if [ -f "../sw_version.h" ]; then
	echo "sw_version.h: OK"
else
	echo "sw_version.h: NOK"
	exit 1
fi

shift $(($OPTIND-1))

f1=$1
f1_b=`basename $f1`

f2=$2
f2_b=`basename $f2`

file_1=${f1_b%.bin}
file_2=${f2_b%.bin}

echo
echo "files used in build:"
echo
echo "$file_1, $file_2"
echo

timestamp=`date +%d%m%y%H%M`

image=suota_image_$timestamp.img

echo "build image"
./mkimage single $file_1.bin ../sw_version.h $file_1.img
./mkimage single $file_2.bin ../sw_version.h $file_2.img

#image_size=`stat --printf=%s $file_1.img`
image_size=`du -b $file_1.img | cut -f1`
# Calculate offest for the second image.
# This offset must be aligned to 64.
# 0x1040 is size of the image and product header.
let image_size=($image_size/64+1)*64+0x1040
printf -v offset "0x%x" "$image_size"
#echo "top($image_size.0/64.0)" | bc
echo "Image size: $image_size"

./mkimage multi spi $file_1.img 0x1000 $file_2.img $offset 0x00000 suota.bin
./mkimage single suota.bin ../sw_version.h $image


#Remove some binaries we create during the process
rm $file_1.img 2>/dev/null
rm $file_2.img 2>/dev/null
rm suota.bin 2>/dev/null

echo
echo
if [ -f $image ]; then
	echo "Multi image done: $image"
else
	echo "Multi image NOT created"
	exit 1
fi
echo
echo
