# Copyright 2021 Xilinx Inc.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from .dpuMcParserBase import *


class xvdpuInstParser(dpuMcParserBase):
    def __init__(self):
        name = "DPUCVDX8G"
        super().__init__(name)
        self.opcode_table = [
            self.inst_desc("LOAD", 0b0000, 5),
            self.inst_desc("SAVE", 0b0100, 4),
            self.inst_desc("CONV", 0b1000, 5),
            self.inst_desc("CONVINIT", 0b1001, 6),
            self.inst_desc("DPTWISE", 0b1010, 5),
            self.inst_desc("DWINIT", 0b1011, 4),
            self.inst_desc("POOLINIT", 0b0110, 2),
            self.inst_desc("POOL", 0b1100, 5),
            self.inst_desc("ELEWINIT", 0b1101, 2),
            self.inst_desc("ELEW", 0b1110, 3),
            self.inst_desc("ALUINIT", 0b0001, 5),
            self.inst_desc("ALU", 0b0010, 5),
            self.inst_desc("END", 0b0111, 1)
        ]

    class Load(LittleEndianStructure):
        _pack_ = 1
        _fields_ = [
            ('bank_addr', c_uint, 14),
            ('bank_id', c_uint, 6),
            ('dpby', c_uint, 4),
            ('dpdon', c_uint, 4),
            ('opcode', c_uint, 4),

            ('jump_read', c_uint, 16),
            ('pad_idx', c_uint, 5),
            ('pad_end', c_uint, 5),
            ('pad_start', c_uint, 5),
            ('r0', c_uint, 1),

            ('channel', c_uint, 12),
            ('mode_avg', c_uint, 2),
            ('length', c_uint, 10),
            ('jump_write', c_uint, 8),

            ('ddr_addr', c_uint, 29),
            ('reg_id', c_uint, 3),

            ('jump_write_endl', c_uint, 14),
            ('block_num', c_uint, 10),
            ('r1', c_uint, 8)
        ]

    class Save(LittleEndianStructure):
        _pack_ = 1
        _fields_ = [
            ('bank_addr', c_uint, 14),
            ('bank_id', c_uint, 6),
            ('dpby', c_uint, 4),
            ('dpdon', c_uint, 4),
            ('opcode', c_uint, 4),

            ('jump_write', c_uint, 16),
            ('r0', c_uint, 16),

            ('channel', c_uint, 12),
            ('r1', c_uint, 2),
            ('length', c_uint, 10),
            ('jump_read', c_uint, 8),

            ('ddr_addr', c_uint, 29),
            ('reg_id', c_uint, 3)
        ]

    def process(self, mc, _debug=False):

        self.debug = _debug

        pos = 0
        load_img_size = 0
        load_para_size = 0
        save_size = 0

        while (pos < len(mc)):
            inst_name, inst_len = self.get_inst(mc, pos, self)

            if self.debug:
                print("pos: %d/%d, inst: %s          " %
                      (pos, len(mc), inst_name), end="\r")
            if inst_name == "LOAD":
                l = self.Load()
                inst = mc[pos:]
                memmove(addressof(l), inst, inst_len)

                length_ = l.length + 1
                chan_ = l.channel + 1
                block_num_ = l.block_num + 1
                bank_id = l.bank_id
                jump_read_ = l.jump_read + 1

                #print("LOAD,%d,%d,%d" % (length_, chan_,block_num_))
                if bank_id < 16:
                    if jump_read_ >= chan_:
                        size = (length_ * chan_)
                    else:
                        size = jump_read_ * (length_) + chan_
                    load_img_size += size
                else:
                    load_para_size += (length_ * chan_ * block_num_)

            elif inst_name == "SAVE":
                s = self.Save()
                inst = mc[pos:]
                memmove(addressof(s), inst, inst_len)

                length_ = s.length + 1
                chan_ = s.channel + 1
                save_size += (length_ * chan_)
                #print("SAVE %d %d" % (length_, chan_))
            elif inst_name == "UNKNOW":
                load_img_size = 0
                load_para_size = 0
                save_size = 0
                break
            else:
                pass

            pos += inst_len

        self.data["load_img_size"] = load_img_size
        self.data["load_para_size"] = load_para_size
        self.data["save_size"] = save_size


register(xvdpuInstParser())
