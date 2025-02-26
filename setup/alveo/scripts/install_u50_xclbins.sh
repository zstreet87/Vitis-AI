#!/bin/bash
# Copyright 2021 Xilinx Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

XCLBIN_URL="https://www.xilinx.com/bin/public/openDownload?filename=DPUCAHX8L_xclbins_1_4_1.tar.gz"
XCLBIN_INSTALLER="/tmp/xclbins.tar.gz"
INSTALLER=tar
INSTALL_PATH=/

wget $XCLBIN_URL -O $XCLBIN_INSTALLER && sudo ${INSTALLER} -xzf $XCLBIN_INSTALLER --directory $INSTALL_PATH && rm $XCLBIN_INSTALLER
sudo rm -rf /opt/xilinx/overlaybins/dpuv3me
sudo ln -sf /opt/xilinx/overlaybins/DPUCAHX8L /opt/xilinx/overlaybins/dpuv3me

XCLBIN_URL="https://www.xilinx.com/bin/public/openDownload?filename=DPUCAHX8H_xclbins_1_4_1.tar.gz"
XCLBIN_INSTALLER="/tmp/xclbins.tar.gz"
INSTALLER=tar
INSTALL_PATH=/

wget $XCLBIN_URL -O $XCLBIN_INSTALLER && sudo ${INSTALLER} -xzf $XCLBIN_INSTALLER --directory $INSTALL_PATH && rm $XCLBIN_INSTALLER
sudo rm -rf /opt/xilinx/overlaybins/dpuv3e
sudo ln -sf /opt/xilinx/overlaybins/DPUCAHX8H /opt/xilinx/overlaybins/dpuv3e

