#
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
#

MODEL_DIR="../vai-rnn-models-1.4.1"

if [[ $TARGET_DEVICE != "U50LV" && $TARGET_DEVICE != "U25" ]]; then
  echo "[ERROR] TARGET_DEVICE should be U50LV or U25"
  return 1;
else
  echo "TARGET_DEVICE = $TARGET_DEVICE"
  device=$(echo $TARGET_DEVICE | awk '{print tolower($0)}')
fi

echo "Get compiled models ..."
if [[ ! -d $MODEL_DIR ]]; then
  wget -nc -O /tmp/vai-rnn-models-1.4.1.tar.gz https://www.xilinx.com/bin/public/openDownload?filename=vai-rnn-models-1.4.1.tar.gz
  tar -xvf /tmp/vai-rnn-models-1.4.1.tar.gz -C ..
fi

echo "Copying the data ..."
mkdir -p data
rm data/*.xmodel 2>/dev/null
cp $MODEL_DIR/$device/lstm_sentiment_detection/*.xmodel data/
cp $MODEL_DIR/float/lstm_sentiment_detection/*.h5 data/
wget -nc https://storage.googleapis.com/tensorflow/tf-keras-datasets/imdb.npz -P data/
wget -nc https://storage.googleapis.com/tensorflow/tf-keras-datasets/imdb_word_index.json -P data/
if [[ ! -f data/IMDB.csv ]]; then
  echo -ne "[ERROR] ./data/IMDB.csv not found.\n"
  echo -ne "Log on to kaggle.com and download the .csv file from "
  echo -ne "https://www.kaggle.com/lakshmi25npathi/imdb-dataset-of-50k-movie-reviews.\n"
  echo -ne "Then keep it as ./data/IMDB.csv\n"
  return 1
fi

echo "Checking xclbin ..."
src_xclbin=../../xclbin/$device/dpu.xclbin
dst_xclbin=/usr/lib/dpu.xclbin
if [[ ! -f $dst_xclbin || `diff -q $dst_xclbin $src_xclbin` ]]; then
  sudo cp $src_xclbin $dst_xclbin
fi

echo "Activate the environment ..."

conda activate rnn-tf-2.0
sudo cp utils/hdf5_format.py ${CONDA_PREFIX}/lib/python3.6/site-packages/tensorflow_core/python/keras/saving/hdf5_format.py
