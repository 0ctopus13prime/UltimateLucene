/*
 *
 * Copyright (c) 2018-2019 Doo Yong Kim. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef SRC_UTIL_FSTBUILDER_H_
#define SRC_UTIL_FSTBUILDER_H_

// TEST
#include <iostream>
// TEST

#include <Codec/CodecUtil.h>
#include <Store/DataInput.h>
#include <Store/DataOutput.h>
#include <Util/ArrayUtil.h>
#include <Util/Etc.h>
#include <Util/Exception.h>
#include <Util/Pack/Paged.h>
#include <Util/Fst.h>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <limits>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

namespace lucene {
namespace core {
namespace util {

template <typename T>
class FstBuilder;

template<typename T>
class NodeHash {
 private:
  PagedGrowableWriter table;
  uint64_t count;
  uint64_t mask;
  Fst<T>* fst;
  typename Fst<T>::Arc scratch_arc;
  std::unique_ptr<FstBytesReader> in;

 private:
/*
  bool NodesEqual(typename FstBuilder<T>::UnCompiledNode* node,
                  const uint64_t address);

  int64_t Hash(typename FstBuilder<T>::UnCompiledNode* node);

  int64_t Hash(const int64_t node);

  void AddNew(const uint64_t address) {
    int64_t pos = (Hash(address) & mask);
    int32_t c = 0;

    while (table.Get(pos)) {
      pos = ((pos + (++c)) & mask);
    }

    table.Set(pos, address);
  }

  void Rehash() {
    NodeHash<T> new_one(PagedGrowableWriter(
                          2 * table.Size(),
                          1 << 30,
                          PackedInts::BitsRequired(count),
                          PackedInts::COMPACT),
                        std::move(*this));
    
    for (uint32_t idx = 0 ; idx < table.Size() ; ++idx) {
      const int64_t address = table.Get(idx);

      if (address != 0) {
        new_one.AddNew(address);  
      }
    }

    operator=(std::move(new_one));
  }
*/

 public:
  NodeHash(Fst<T>* fst,
           std::unique_ptr<FstBytesReader>&& in)
    : table(16, 1 << 27, 8, PackedInts::COMPACT),
      count(0),
      mask(15),
      fst(fst),
      scratch_arc(),
      in(std::forward<std::unique_ptr<FstBytesReader>>(in)) {
  }

  NodeHash(const NodeHash<T>& other) = delete;

  NodeHash<T>& operator=(const NodeHash<T>& other) = delete;

  NodeHash(NodeHash<T>&& other)
    : table(std::move(other.table)),
      mask(other.mask),
      fst(other.fst),
      scratch_arc(other.scratch_arc),
      in(std::move(other.in)) {
  }

  NodeHash<T>& operator=(NodeHash<T>&& other) {
    if (this != &other) {
      table = std::move(other.table);
      count = other.count;
      mask = other.mask;
      fst = other.fst;
      scratch_arc = other.scratch_arc;
      in = std::move(other.in);
    }

    return *this;
  }

  int64_t Add(FstBuilder<T>* builder,
              typename FstBuilder<T>::UnCompiledNode* node_in) {
    const int64_t h = Hash(node_in);
    // std::cout << "Hash, h -> " << h << std::endl;
    int64_t pos = (h & mask);
    int32_t c = 0;

    while (true) {
      const int64_t v = table.Get(pos);
      // std::cout << "Finding ..." << std::endl;
      if (v == 0) {
        // Freeze & Add
        // std::cout << "Now really freeze node" << std::endl;
        const int64_t node = builder->AddNode(node_in);
        // std::cout << "Now really freeze done" << std::endl;

        if (Hash(node) != h) {
          Debug(node_in);
          Debug(node);
          assert(Hash(node) == h);
        }
        // assert(Hash(node) == h);
        count++;

        // std::cout << "Put addr into the table, addr - " << node << std::endl;
        table.Set(pos, node);
        if (count > (2 * table.Size()) / 3) {
          // std::cout << "Start rehash" << std::endl;
          Rehash();
        }
        return node;
      } else if (NodesEqual(node_in, v)) {
        return v;
      }

      pos = ((pos + (++c)) & mask);
    }
  }

  // TEST
  bool NodesEqual(typename FstBuilder<T>::UnCompiledNode* node,
                  const uint64_t address);

  int64_t Hash(typename FstBuilder<T>::UnCompiledNode* node);

  int64_t Debug(typename FstBuilder<T>::UnCompiledNode* node);

  int64_t Hash(const int64_t node);

  int64_t Debug(const int64_t node);

  void AddNew(const uint64_t address,
              const uint32_t new_mask,
              PagedGrowableWriter& new_table) {
    int64_t pos = (Hash(address) & new_mask);

    // std::cout << "AddNew, pos - " << pos
    //           << ", addr - " << address << std::endl;

    uint32_t cnt = 0;
    for (int64_t c = 1 ; new_table.Get(pos) != 0 ; ++c) {
      // std::cout << "In new table, pos - " << pos
      //           << ", got -" << new_table.Get(pos) << std::endl;
      // Quadratic probe
      pos = ((pos + c) & new_mask);
      assert(++cnt < new_table.Size());
    }
    new_table.Set(pos, address);
    // std::cout << "New insertion is done" << std::endl;
  }

  void Rehash() {
    PagedGrowableWriter new_table(2 * table.Size(),
                                  1 << 30,
                                  PackedInts::BitsRequired(count),
                                  PackedInts::COMPACT);
    uint64_t new_mask = (new_table.Size() - 1); 

    // std::cout << "NodeHash is rehashing ... new table size - " << new_table.Size() << std::endl;
    for (uint32_t idx = 0 ; idx < table.Size() ; ++idx) {
      if (const int64_t address = table.Get(idx)) {
        // std::cout << "Insert address - " << address << std::endl;
        AddNew(address, new_mask, new_table);

      }
    }

    table = std::move(new_table);
    mask = new_mask;
    // std::cout << "Rehashing is done" << std::endl;
  }
  // TEST
};  // NodeHash

template<typename T>
class FstBuilder {
 friend class NodeHash<T>;

 public:
  class Node;
  class UnCompiledNode;
  class CompiledNode;
  class Arc;

 private:
  std::unique_ptr<NodeHash<T>> dedup_hash;
  Fst<T> fst;
  uint32_t min_suffix_count1;
  uint32_t min_suffix_count2;
  BytesStore& bytes;
  std::vector<uint32_t> reused_bytes_per_arc;
  uint32_t share_max_tail_length;
  lucene::core::util::IntsRefBuilder last_input;
  std::vector<UnCompiledNode*> frontier;
  std::list<CompiledNode> compiled_node_pool;
  std::list<UnCompiledNode> uncompiled_node_pool;
  uint64_t last_frozen_node;
  uint64_t arc_count;
  uint64_t node_count;
  bool do_share_non_singleton_nodes;
  bool allow_array_arcs;

 private:
  void WriteLabel(lucene::core::store::DataOutput* out, const int32_t v) {
    assert(v >= 0);
    if (fst.input_type == FST_INPUT_TYPE::BYTE1) {
      assert(v <= 255);
      out->WriteByte(static_cast<char>(v));
    } else if (fst.input_type == FST_INPUT_TYPE::BYTE2) {
      assert(v <= 65535);
      out->WriteInt16(static_cast<int16_t>(v));
    } else {
      out->WriteInt32(v);
    }
  }

  int64_t AddNode(typename FstBuilder<T>::UnCompiledNode* node_in) {
    if (node_in->arcs.empty()) {
      return (node_in->is_final ? Fst<T>::FINAL_END_NODE :
                                  Fst<T>::NON_FINAL_END_NODE);
    }

    const uint64_t start_address = bytes.GetPosition();
    // std::cout << "Start address -> " << start_address << std::endl;
    const bool do_fixed_array = ShouldExpand(node_in);
    // std::cout << "do_fixed_array -> " << std::boolalpha << do_fixed_array << std::endl;

    if (do_fixed_array) {
      reused_bytes_per_arc.reserve(ArrayUtil::Oversize(node_in->arcs.size()));
    }

    arc_count += node_in->arcs.size();
    // std::cout << "Arc count -> " << arc_count << std::endl;
    // std::cout << "Num arcs of the node - " << node_in->arcs.size()
    //           << std::endl;

    const uint32_t last_arc = (node_in->arcs.size() - 1);
    uint64_t last_arc_start = bytes.GetPosition();
    uint32_t max_bytes_per_arc = 0;

    // std::cout << "last_arc -> " << last_arc
    //           << ", last_arc_start -> " << last_arc_start << std::endl;

    for (uint32_t arc_idx = 0 ; arc_idx < node_in->arcs.size() ; ++arc_idx) {
      // std::cout << "\nProcessing [" << arc_idx << "] arc" << std::endl;      

      Arc& arc = node_in->arcs[arc_idx];
      CompiledNode* target = dynamic_cast<CompiledNode*>(arc.target);
      const bool target_has_arcs = (target->node > 0);

      // Fst<T>::BIT_ARC_HAS_FINAL_OUTPUT
      uint32_t flags = static_cast<uint32_t>(arc.is_final &&
                       !fst.outputs->IsNoOutput(arc.next_final_output));

      // std::cout << "BIT_ARC_HAS_FINAL_OUTPUT, Flags -> " << (flags & 1) << std::endl;

      // Fst<T>::BIT_ARC_HAS_OUTPUT
      flags =
        ((flags << 1) | static_cast<uint32_t>(
                        !(fst.outputs->IsNoOutput(arc.output))));

      // std::cout << "BIT_ARC_HAS_OUTPUT, Flags -> " << (flags & 1) << std::endl;
      // std::cout << "output != nullptr? " << std::boolalpha
      //           << (arc.output.Ints() != nullptr) << std::endl;

      // Fst<T>::BIT_STOP_NODE 
      flags = ((flags << 1) |
               static_cast<uint32_t>(!target_has_arcs));

      // std::cout << "target's node -> " << target->node << std::endl;
      // std::cout << "BIT_STOP_NODE, Flags -> " << (flags & 1) << std::endl;

      // Fst<T>::BIT_TARGET_NEXT
      flags = ((flags << 1) |
               static_cast<uint32_t>(last_frozen_node == target->node &&
                                     !do_fixed_array));

      // std::cout << "BIT_TARGET_NEXT, Flags -> " << (flags & 1) << std::endl;

      // Fst<T>::BIT_LAST_ARC
      flags = ((flags << 1) |
               static_cast<uint32_t>(arc_idx == last_arc));

      // std::cout << "BIT_LAST_ARC, Flags -> " << (flags & 1) << std::endl;

      // Fst<T>::BIT_FINAL_ARC
      flags = ((flags << 1) | static_cast<uint32_t>(arc.is_final));

      // std::cout << "BIT_FINAL_ARC, Flags -> " << (flags & 1) << std::endl;
      // std::cout << "Final flags -> " << flags << std::endl;

      if (!arc.is_final) {
        assert(fst.outputs->IsNoOutput(arc.next_final_output));
      }

      bytes.WriteByte(static_cast<char>(flags));

      // std::cout << "Write label, label -> " << arc.label << std::endl;
      WriteLabel(&bytes, arc.label);

      if (!fst.outputs->IsNoOutput(arc.output)) {
        // std::cout << "Because arc.output is not a no-output value. Write it's value"
        //           << ", Length - " << arc.output.Length() << std::endl;
        fst.outputs->Write(arc.output, &bytes);
      }

      if (!fst.outputs->IsNoOutput(arc.next_final_output)) {
        // std::cout << "Because arc.next_final_output is not a no-output value. Write it's value"
        //           << ", Length - " << arc.next_final_output.Length() << std::endl;
        fst.outputs->WriteFinalOutput(arc.next_final_output, &bytes);
      }

      if (target_has_arcs && !(flags & Fst<T>::BIT_TARGET_NEXT)) {
        // std::cout << "Write target's node, as it's BIT_TARGET_NEXT - " << target->node << std::endl;
        assert(target->node > 0);
        bytes.WriteVInt64(target->node);
      }

      if (do_fixed_array) {
        reused_bytes_per_arc[arc_idx] =
          static_cast<uint32_t>(bytes.GetPosition() - last_arc_start);
        // std::cout << "In case `do_fixed_array`, bytes -> " << reused_bytes_per_arc[arc_idx] << std::endl;

        last_arc_start = bytes.GetPosition();
        // std::cout << "last_arc_start -> " << last_arc_start << std::endl;

        max_bytes_per_arc =
          std::max(max_bytes_per_arc, reused_bytes_per_arc[arc_idx]);
        // std::cout << "max_bytes_per_arc -> " << max_bytes_per_arc << std::endl;
      }

    }  // End for

    // std::cout << "Do fixed array -> " << std::boolalpha << do_fixed_array << std::endl;
    if (do_fixed_array) {
      // std::cout << "Doing fixed array" << std::endl;
      // Header(byte) + Num arcs(vint) + Num bytes(vint)
      const uint32_t MAX_HEADER_SIZE = 11;
      assert(max_bytes_per_arc > 0);
      // 2nd pass just "expands" all arcs to take up a fixed byte size

      char header[MAX_HEADER_SIZE];
      lucene::core::store::ByteArrayReferenceDataOutput bad(header,
                                                            MAX_HEADER_SIZE);
      // Write a "false" first arc:
      bad.WriteByte(Fst<T>::ARCS_AS_FIXED_ARRAY);
      bad.WriteVInt32(node_in->arcs.size());
      bad.WriteVInt32(max_bytes_per_arc);
      const uint32_t header_len = bad.GetPosition();

      const uint32_t fixed_array_start = (start_address + header_len);
      // std::cout << "fixed_array_start -> " << fixed_array_start << std::endl;

      // Expand the arcs in place, backwards
      uint64_t src_pos = bytes.GetPosition();
      uint64_t dest_pos = 
        (fixed_array_start + node_in->arcs.size() * max_bytes_per_arc);

      // std::cout << "src_pos -> " << src_pos
      //           << ", dest_pos -> " << dest_pos
      //           << ", max_bytes_per_arc -> " << max_bytes_per_arc << std::endl;

      if (dest_pos > src_pos) {
        bytes.SkipBytes(static_cast<uint32_t>(dest_pos - src_pos));
        for (int32_t arc_idx = node_in->arcs.size() - 1 ;
             arc_idx >= 0 ;
             --arc_idx) {
          // std::cout << "arc_id - " << arc_idx << std::endl;
          dest_pos -= max_bytes_per_arc;
          src_pos -= reused_bytes_per_arc[arc_idx];

          // std::cout << "src_pos -> " << src_pos
          //           << ", dest_pos -> " << dest_pos
          //           << ", reused_bytes_per_arc -> " << reused_bytes_per_arc[arc_idx]
          //           << ", max_bytes_per_arc -> " << max_bytes_per_arc << std::endl;
          // std::cout << arc_idx << "'s bytes -> " << reused_bytes_per_arc[arc_idx] << std::endl;
          if (src_pos != dest_pos) {
            assert(dest_pos > src_pos);
            bytes.CopyBytes(src_pos, dest_pos, reused_bytes_per_arc[arc_idx]);
          }
        }  // End for
      }  // End if

      // Now write the header
      // std::cout << "Write the header start" << std::endl;
      bytes.WriteBytes(start_address, header, 0, header_len);
      // std::cout << "Write the header is done" << std::endl;
    }  // End if

    // std::cout << "\nArc process is done" << std::endl;

    const uint64_t this_node_address = (bytes.GetPosition() - 1);
    // std::cout << "This node's addr - " << this_node_address << std::endl;
    bytes.Reverse(start_address, this_node_address);
    node_count++;

    return this_node_address;
  }

  bool ShouldExpand(typename FstBuilder<T>::UnCompiledNode* node) {
    return allow_array_arcs &&
           ((node->depth <= Fst<T>::FIXED_ARRAY_SHALLOW_DISTANCE &&
             node->arcs.size() >= Fst<T>::FIXED_ARRAY_NUM_ARCS_SHALLOW) ||
            (node->arcs.size() >= Fst<T>::FIXED_ARRAY_NUM_ARCS_DEEP));
  }

  CompiledNode* CompileNode(UnCompiledNode* node_in,
                            const uint32_t tail_length) {
    int64_t node;
    const uint64_t bytes_pos_start = bytes.GetPosition();

    // std::cout << "Compile Node, arcs - " << node_in->arcs.size()
    //           << ", tail length - " << tail_length
    //           << ", bytes pos start - " << bytes_pos_start << std::endl;

    if (dedup_hash &&
        (do_share_non_singleton_nodes || node_in->arcs.size() <= 1) &&
        tail_length <= share_max_tail_length) {
      if (node_in->arcs.empty()) {
        // std::cout << "Add node" << std::endl;
        node = AddNode(node_in); 
        last_frozen_node = node;
      } else {
        // std::cout << "Add hash" << std::endl;
        node = dedup_hash->Add(this, node_in);
        // std::cout << "Add hash done" << std::endl;
      }
    } else {
      // std::cout << "No hash, Add node" << std::endl;
      node = AddNode(node_in);
    }

    // std::cout << "\nNode -> " << node << std::endl;

    const uint64_t bytes_pos_end = bytes.GetPosition();
    if (bytes_pos_end != bytes_pos_start) {
      last_frozen_node = node;
    }

    return CompiledNode::New(this, node); 
  }
  
  void FreezeTail(const uint32_t prefix_len_plus1) {
    const uint32_t down_to = std::max(1U, prefix_len_plus1);
    // std::cout << "Last input's length - " << last_input.Length() << std::endl;

    for (uint32_t idx = last_input.Length() ; idx >= down_to ; --idx) {
      // std::cout << "FreezeeTail, idx - " << idx << std::endl;

      bool do_prune = false;
      bool do_compile = false;

      UnCompiledNode* node = frontier[idx];
      UnCompiledNode* parent = frontier[idx - 1]; 

      if (node->input_count < min_suffix_count1) {
        do_prune = true;
        do_compile = true;
      } else if(idx > prefix_len_plus1) {
        // Prune if parent's inputCount is less than suffixMinCount2
        if (parent->input_count < min_suffix_count2 ||
            (min_suffix_count2 == 1 && parent->input_count == 1 && idx > 1)) {
          // My parent, about to be compiled, doesn't make the cut, so
          // I'm definitely pruned 

          // If minSuffixCount2 is 1, we keep only up
          // until the 'distinguished edge', ie we keep only the
          // 'divergent' part of the FST. if my parent, about to be
          // compiled, has inputCount 1 then we are already past the
          // distinguished edge.  NOTE: this only works if
          // the FST outputs are not "compressible" (simple
          // ords ARE compressible).
          do_prune = true;
        } else {
          // my parent, about to be compiled, does make the cut, so
          // I'm definitely not pruned 
          do_prune = false;
        }
        do_compile = true;
      } else {
        // If pruning is disabled (count is 0) we can always
        // compile current node
        do_compile = (min_suffix_count2 == 0);
      }

      if (node->input_count < min_suffix_count2 ||
          (min_suffix_count2 == 1 && node->input_count == 1 && idx > 1)) {
        // Drop all arcs
        for (Arc& arc : node->arcs) {
          dynamic_cast<UnCompiledNode*>(arc.target)->Clear(); 
        }

        node->arcs.clear();
      }

      if (do_prune) {
        // This node doesn't make it -- Deref it
        node->Clear();
        parent->DeleteLast(last_input[idx - 1], node);
      } else {
        if (min_suffix_count2 != 0) {
          CompileAllTargets(node, last_input.Length() - idx);
        }

        // We "fake" the node as being final if it has no
        // outgoing arcs; in theory we could leave it
        // as non-final (the FST can represent this), but
        // FSTEnum, Util, etc., have trouble w/ non-final
        // dead-end states: 
        const bool is_final = (node->is_final || node->arcs.empty());

        if (do_compile) {
          // This node makes it and we now compile it.  first,
          // compile any targets that were previously
          // undecided:
          
          // std::cout << "Do compile" << std::endl;
          CompiledNode* compiled_node =
            CompileNode(node, 1 + last_input.Length() - idx); 
          // std::cout << "Compile done" << std::endl;
          // std::cout << "Bytes pos - " << bytes.GetPosition() << std::endl;

          parent->ReplaceLast(last_input[idx - 1],
                              compiled_node,
                              std::move(node->output),
                              is_final);
          node->Clear();
        } else {
          // std::cout << "No compile, Just install next final output + is_final" << std::endl;
          // ReplaceLast just to install
          // next_final_output/is_final onto the arc
          parent->ReplaceLast(last_input[idx - 1],
                              node,
                              std::move(node->output),
                              is_final);

          // This node will stay in play for now, since we are
          // undecided on whether to prune it.  later, it
          // will be either compiled or pruned, so we must
          // allocate a new node:
          frontier[idx] = UnCompiledNode::New(this, idx);
        }
      }
    }
  }

  void CompileAllTargets(UnCompiledNode* node, const uint32_t tail_length) {
    for (Arc& arc : node->arcs) {
      if (UnCompiledNode* n = dynamic_cast<UnCompiledNode*>(arc.target)) {
        if (n->arcs.empty()) {
          arc.is_final = n->is_final = true;
        }

        arc.target = CompileNode(n, tail_length - 1);
      }
    } 
  }

 public:
  FstBuilder(const FST_INPUT_TYPE input_type,
          std::unique_ptr<Outputs<T>>&& outputs)
    : FstBuilder(input_type,
              0,
              0,
              true,
              true,
              std::numeric_limits<int32_t>::max(),
              std::forward<std::unique_ptr<Outputs<T>>>(outputs),
              true,
              15) {
  }

/*
  std::unique_ptr<NodeHash<T>> dedup_hash;
  Fst<T> fst;
  uint32_t min_suffix_count1;
  uint32_t min_suffix_count2;
  BytesStore& bytes;
  std::vector<uint32_t> reused_bytes_per_arc;
  uint32_t share_max_tail_length;
  lucene::core::util::IntsRefBuilder last_input;
  std::vector<UnCompiledNode*> frontier;
  std::vector<CompiledNode> compiled_node_pool;
  std::vector<UnCompiledNode> uncompiled_node_pool;
  uint64_t last_frozen_node;
  uint64_t arc_count;
  uint64_t node_count;
  bool do_share_non_singleton_nodes;
  bool allow_array_arcs;
*/

  FstBuilder(const FST_INPUT_TYPE input_type,
          const uint32_t min_suffix_count1,
          const uint32_t min_suffix_count2,
          const bool do_share_suffix,
          const bool do_share_non_singleton_nodes,
          const uint32_t share_max_tail_length,
          std::unique_ptr<Outputs<T>>&& outputs,
          const bool allow_array_arcs,
          const uint32_t bytes_page_bits)
    : dedup_hash(),
      fst(input_type,
          std::forward<std::unique_ptr<Outputs<T>>>(outputs),
          bytes_page_bits),
      min_suffix_count1(min_suffix_count1),
      min_suffix_count2(min_suffix_count2),
      bytes(*fst.bytes),
      reused_bytes_per_arc(),
      share_max_tail_length(share_max_tail_length),
      last_input(),
      frontier(),
      compiled_node_pool(),
      uncompiled_node_pool(),
      last_frozen_node(0),
      arc_count(0),
      node_count(0),
      do_share_non_singleton_nodes(do_share_non_singleton_nodes),
      allow_array_arcs(allow_array_arcs) {
    if (do_share_suffix) {
      dedup_hash =
        std::make_unique<NodeHash<T>>(&fst,
                                      bytes.GetReverseReader(false));
    }

    reused_bytes_per_arc.reserve(4);

    const uint32_t frontier_rsv_size = 20;
    frontier.reserve(frontier_rsv_size);
    for (uint32_t idx = 0 ; idx < frontier_rsv_size ; ++idx) {
      frontier.push_back(UnCompiledNode::New(this, idx));
    }

    //std::cout << "FstBuilder(), frontier's size -> " << frontier.size() << std::endl; 
    //std::cout << "FstBuilder(), uncompiled_node_pool's size -> " << uncompiled_node_pool.size() << std::endl; 
    //std::cout << "FstBuilder(), outputs -> " << fst.outputs->get() << std::endl;
  }

  FstBuilder(const FstBuilder&) = delete;
  FstBuilder(FstBuilder&&) = delete;
  FstBuilder& operator=(const FstBuilder&) = delete;
  FstBuilder& operator=(FstBuilder&&) = delete;

  uint64_t GetTermCount() const noexcept {
    return frontier[0]->input_count; 
  }

  uint64_t GetNodeCount() const noexcept {
    return (1 + node_count);
  }

  uint64_t GetArcCount() const noexcept {
    return arc_count;
  }

  uint64_t GetMappedStateCount() const noexcept {
    return (dedup_hash ? node_count : 0L);
  }

  void Add(lucene::core::util::IntsRef&& input, T&& output) {
    assert(last_input.Length() == 0 || !(input < last_input.Get()));

    if (input.Length() == 0) {
      frontier[0]->input_count++;
      frontier[0]->is_final = true;
      fst.SetEmptyOutput(std::forward<T>(output));
      return;
    }

    const int32_t* pos1 = (last_input.Ints() + last_input.Offset());
    const int32_t* pos2 = (input.Ints() + input.Offset());

    uint32_t prefix_len_plus1 = 0;
    const uint32_t prefix_cmp_until =
      std::min(last_input.Length(), input.Length());
    auto frontier_it = frontier.begin();

    do {
      (*frontier_it++)->input_count++;
    } while(prefix_len_plus1++ < prefix_cmp_until &&
            *pos1++ == *pos2++);

    if (frontier.size() < input.Length() + 1) {
      const uint32_t until = ArrayUtil::Oversize(input.Length() + 1);
      for (uint32_t idx = frontier.size() ; idx < until ; ++idx) {
        frontier.push_back(UnCompiledNode::New(this, idx));
      }
    } 

    // Minimize/Compile states from previous input's
    // orphan'd suffix
    // std::cout << "Start freeze tail, Prefix + 1 - " << prefix_len_plus1 << std::endl;
    FreezeTail(prefix_len_plus1);
    // std::cout << "End freeze tail" << std::endl;

    for (uint32_t idx = prefix_len_plus1 ; idx <= input.Length() ; ++idx) {
      frontier[idx - 1]->AddArc(input.Ints()[input.Offset() + idx - 1],
                                frontier[idx]);
      frontier[idx]->input_count++;
    }

    UnCompiledNode* last_node = frontier[input.Length()];
    if (last_input.Length() != input.Length() ||
        prefix_len_plus1 != input.Length() + 1) {
      last_node->is_final = true;
      fst.outputs->MakeNoOutput(last_node->output);
    }

    // Push conflicting outputs forward, only as far as needed
    for (uint32_t idx = 1 ; idx < prefix_len_plus1 ; ++idx) {
      UnCompiledNode* node = frontier[idx];
      UnCompiledNode* parent_node = frontier[idx - 1];

      T& last_output =
        parent_node->GetLastOutput(input.Ints()[input.Offset() + idx - 1]);

      if (!fst.outputs->IsNoOutput(last_output)) {
        // Calculate common prefix between output and last_output
        const uint32_t prefix_len = fst.outputs->PrefixLen(output, last_output);

        // Propagate last input's suffix to all arcs of node
        node->PrependOutput(
          fst.outputs->SuffixReference(last_output, prefix_len));

        // Drop suffix from last_output
        fst.outputs->DropSuffix(last_output, prefix_len);

        // Drop common prefix from output
        fst.outputs->DropPrefix(output, prefix_len);
      }
    }

    if (last_input.Length() == input.Length() &&
        prefix_len_plus1 == (1 + input.Length())) {
      // Same input more than 1 time in a row, mapping to
      // multiple outputs
      last_node->output = fst.outputs->Merge(last_node->output, output);
    } else {
      // This new arc is private to this new input; 
      // set its arc output to the leftover output:
      frontier[prefix_len_plus1 - 1]->SetLastOutput(
        input.Ints()[input.Offset() + prefix_len_plus1 - 1],
        std::forward<T>(output));
    }

    // Save last input
    last_input.InitInts(std::forward<IntsRef>(input));

    // std::cout << "Bytes pos - " << bytes.GetPosition() << std::endl;
  }

  // TEST
  NodeHash<T>& GetNodeHash() {
    return *(dedup_hash.get());
  }
  // TEST

  Fst<T>* Finish() {
    UnCompiledNode* root = frontier[0];
    std::cout << "FstBuider, Before freeze root, Bytes pos - " << bytes.GetPosition() << std::endl;
    FreezeTail(0);
    std::cout << "FstBuider, Freezed root, Bytes pos - " << bytes.GetPosition() << std::endl;
    if (root->input_count < min_suffix_count1 ||
        root->input_count < min_suffix_count2 ||
        root->arcs.empty()) {
      std::cout << "[Finish] ====================================" << std::endl;
      std::cout << "root.input_count -> " << root->input_count << std::endl;
      std::cout << "root.num_arcs -> " << root->arcs.size() << std::endl;
      std::cout << "min_suffix_count1 -> " << min_suffix_count1 << std::endl;
      std::cout << "min_suffix_count2 -> " << min_suffix_count2 << std::endl;
      std::cout << "fst.AcceptEmptyOutput -> " << std::boolalpha << fst.AcceptEmptyOutput()
                << std::endl;
      std::cout << "[Finish] ====================================" << std::endl;
               
      if (!fst.AcceptEmptyOutput() ||
          (min_suffix_count1 > 0 || min_suffix_count2 > 0)) {
        return nullptr;
      }
    } else if(min_suffix_count2 != 0) {
      CompileAllTargets(root, last_input.Length());
    }

    fst.Finish(CompileNode(root, last_input.Length())->node);
    root->Clear();

    return &fst;
  }
};  // FstBuilder<T>

template<typename T>
class FstBuilder<T>::Node {
 public:
  Node() = default;

  virtual ~Node() = default;

  virtual bool IsCompiled() const noexcept = 0;
};  // FstBuilder<T>::Node

template<typename T>
class FstBuilder<T>::Arc {
 public:
  int32_t label;
  Node* target;
  bool is_final;
  T output;
  T next_final_output;

  Arc() = default;

  Arc(const Arc& other) = delete;

  Arc& operator=(const Arc& other) = delete;

  Arc(Arc&& other)
    : label(other.label),
      target(other.target),
      is_final(other.is_final),
      output(std::move(other.output)),
      next_final_output(std::move(other.next_final_output)) {
  }

  Arc& operator=(Arc&& other) {
    if (this != &other) {
      label = other.label;
      target = other.target;
      is_final = other.is_final;
      output = std::move(other.output);
      next_final_output = std::move(other.next_final_output);
    }

    return *this;
  }
};  // FstBuilder<T>::Arc

template<typename T>
class FstBuilder<T>::UnCompiledNode : public FstBuilder<T>::Node {
 private:
  Outputs<T>* outputs;

 public:
  std::vector<Arc> arcs;
  uint64_t input_count;
  T output;
  uint32_t depth;
  bool is_final;

 public:
  static UnCompiledNode* New(FstBuilder<T>* builder, const uint64_t depth) {
    builder->uncompiled_node_pool.push_back(
      UnCompiledNode(builder->fst.outputs.get(), depth)); 
    return &(builder->uncompiled_node_pool.back());
  }

  UnCompiledNode(Outputs<T>* outputs, const uint32_t depth)
    : outputs(outputs),
      arcs(),
      input_count(0),
      output(),
      depth(depth),
      is_final(false) {
    outputs->MakeNoOutput(output);
  }

  UnCompiledNode(const UnCompiledNode& other) = delete;

  UnCompiledNode& operator=(const UnCompiledNode& other) = delete;

  UnCompiledNode(UnCompiledNode&& other)
    : outputs(other.outputs),
      arcs(std::move(other.arcs)),
      input_count(other.input_count),
      output(std::move(other.output)),
      depth(other.depth),
      is_final(other.is_final) {
  }

  UnCompiledNode& operator=(UnCompiledNode&& other) {
    if (this != &other) {
      outputs = other.outputs;
      arcs = std::move(other.arcs);
      input_count = other.input_count;
      output = std::move(other.output);
      depth = other.depth;
      is_final = other.is_final;
    }

    return *this;
  }

  bool IsCompiled() const noexcept {
    return false;
  }

  void Clear() noexcept {
    arcs.clear();
    input_count = 0;
    is_final = false;
    outputs->MakeNoOutput(output);
  }

  T& GetLastOutput(const int32_t label_to_match) noexcept {
    return arcs.back().output;
  }

  void AddArc(const int32_t label, Node* target) {
    arcs.push_back(Arc());
    Arc& arc = arcs.back();
    arc.label = label;
    arc.target = target;
    outputs->MakeNoOutput(arc.output);
    outputs->MakeNoOutput(arc.next_final_output);
    arc.is_final = false;
  }

  void ReplaceLast(const int32_t label_to_match,
                   Node* target,
                   T&& next_final_output,
                   const bool is_final) {
    assert(!arcs.empty());
    Arc& arc = arcs.back();
    assert(arc.label == label_to_match);
    arc.target = target;
    arc.next_final_output = std::forward<T>(next_final_output);
    arc.is_final = is_final;
  }

  void DeleteLast(const int32_t label, Node* target) noexcept {
    arcs.pop_back();
  }

  void SetLastOutput(const int32_t label_to_match, T&& new_output) {
    Arc& arc = arcs.back();
    arc.output = std::forward<T>(new_output);
  }

  void PrependOutput(const T& prefix) {
    for (uint32_t arc_idx = 0 ; arc_idx < arcs.size() ; ++arc_idx) {
      outputs->Prepend(prefix, arcs[arc_idx].output);
    }

    if (is_final) {
      outputs->Prepend(prefix, output);
    }
  }
};  // FstBuilder<T>::UnCompiledNode

template<typename T>
class FstBuilder<T>::CompiledNode: public FstBuilder<T>::Node {
 public:
  const int64_t node;

 private:
  explicit CompiledNode(const int64_t node)
    : node(node) {
  }

 public:
  static CompiledNode* New(FstBuilder<T>* builder, const int64_t node) {
    builder->compiled_node_pool.push_back(CompiledNode(node)); 
    return &(builder->compiled_node_pool.back());
  }

  bool IsCompiled() const noexcept {
    return true;
  }

  CompiledNode(const CompiledNode& other)
    : node(other.node) {
  }

  CompiledNode& operator=(const CompiledNode& other) noexcept {
    node = other.node;
    return *this;
  }
};  // FstBuilder<T>::CompiledNode

/**
 *  NodeHash
 */ 
template <typename T>
bool NodeHash<T>::NodesEqual(typename FstBuilder<T>::UnCompiledNode* node,
                             const uint64_t address) {
  fst->ReadFirstRealTargetArc(address, scratch_arc, in.get());
  if (scratch_arc.bytes_per_arc != 0 &&
      node->arcs.size() != scratch_arc.num_arcs) {
    return false; 
  }

  uint32_t arc_upto = 0;
  for (typename FstBuilder<T>::Arc& arc : node->arcs) {
    if (arc.label != scratch_arc.label ||
        arc.output != scratch_arc.output ||
        dynamic_cast<typename FstBuilder<T>::CompiledNode*>(arc.target)->node
          != scratch_arc.target ||
        arc.next_final_output != scratch_arc.next_final_output ||
        arc.is_final != scratch_arc.IsFinal()) {
      return false;
    } else if (scratch_arc.IsLast()) {
      return (arc_upto == (node->arcs.size() - 1));
    }

    arc_upto++;
    fst->ReadNextRealArc(scratch_arc, in.get());
  }

  return false;
}

template <typename T>
int64_t NodeHash<T>::Hash(typename FstBuilder<T>::UnCompiledNode* node) {
  const uint32_t PRIME = 31;  
  int64_t h = 0;

  for (typename FstBuilder<T>::Arc& arc : node->arcs) {
    h = PRIME * h + arc.label;
    const int64_t n =
      dynamic_cast<typename FstBuilder<T>::CompiledNode*>(arc.target)->node;
    h = PRIME * h + static_cast<int32_t>(n ^ (n >> 32));
    h = PRIME * h + arc.output.HashCode();
    h = PRIME * h + arc.next_final_output.HashCode();
    if (arc.is_final) {
      h += 17;
    }
  }

  return (h & std::numeric_limits<int64_t>::max());
}

template <typename T>
int64_t NodeHash<T>::Debug(typename FstBuilder<T>::UnCompiledNode* node) {
  const uint32_t PRIME = 31;  
  int64_t h = 0;

  std::cout << "******************************" << std::endl;
  for (typename FstBuilder<T>::Arc& arc : node->arcs) {
    std::cout << "\nLabel - " << arc.label << std::endl;

    h = PRIME * h + arc.label;
    const int64_t n =
      dynamic_cast<typename FstBuilder<T>::CompiledNode*>(arc.target)->node;
    std::cout << "Target - " << n << std::endl;
    h = PRIME * h + static_cast<int32_t>(n ^ (n >> 32));
    h = PRIME * h + arc.output.HashCode();
    std::cout << "Output hash - " << arc.output.HashCode() << std::endl;
    h = PRIME * h + arc.next_final_output.HashCode();
    std::cout << "Next final output hash - " << arc.next_final_output.HashCode() << std::endl;
    std::cout << "Final ? " << std::boolalpha << arc.is_final << std::endl;
    if (arc.is_final) {
      h += 17;
    }
  }

  std::cout << "h - " << h << std::endl;
  std::cout << "******************************" << std::endl;
  return (h & std::numeric_limits<int64_t>::max());
}

template <typename T>
int64_t NodeHash<T>::Hash(const int64_t node) {
  const uint32_t PRIME = 31;
  int64_t h = 0;
  // std::cout << "Start ReadFirstRealTargetArc, node -> " << node << std::endl;
  fst->ReadFirstRealTargetArc(node, scratch_arc, in.get());
  // std::cout << "ReadFirstRealTargetArc done" << std::endl;
  while (true) {
    h = PRIME * h + scratch_arc.label;
    h = PRIME * h +
        static_cast<int32_t>(scratch_arc.target ^ (scratch_arc.target >> 32));
    h = PRIME * h + scratch_arc.output.HashCode();
    h = PRIME * h + scratch_arc.next_final_output.HashCode();
    if (scratch_arc.IsFinal()) {
      h += 17;
    }

    if (scratch_arc.IsLast()) {
      break;
    }

    fst->ReadNextRealArc(scratch_arc, in.get());
  }

  return (h & std::numeric_limits<int64_t>::max());
}

template <typename T>
int64_t NodeHash<T>::Debug(const int64_t node) {
  const uint32_t PRIME = 31;
  int64_t h = 0;
  fst->ReadFirstRealTargetArc(node, scratch_arc, in.get());

  std::cout << "====================" << std::endl; 
  while (true) {
    std::cout << "\nLabel - " << scratch_arc.label << std::endl;
    std::cout << "Target - " << scratch_arc.target << std::endl;
    std::cout << "Next arc - " << scratch_arc.next_arc << std::endl;
    std::cout << "Flags - " << static_cast<int32_t>(scratch_arc.flags) << std::endl;
    std::cout << "Pos arcs start - " << scratch_arc.pos_arcs_start << std::endl;
    std::cout << "Output hash - " << scratch_arc.output.HashCode() << std::endl;
    std::cout << "Next final output hash - " << scratch_arc.next_final_output.HashCode() << std::endl;
    std::cout << "Final ? " << std::boolalpha << scratch_arc.IsFinal() << std::endl;
    std::cout << "Last ? " << std::boolalpha << scratch_arc.IsLast() << std::endl;
    h = PRIME * h + scratch_arc.label;
    h = PRIME * h +
        static_cast<int32_t>(scratch_arc.target ^ (scratch_arc.target >> 32));
    h = PRIME * h + scratch_arc.output.HashCode();
    h = PRIME * h + scratch_arc.next_final_output.HashCode();
    if (scratch_arc.IsFinal()) {
      h += 17;
    }

    if (scratch_arc.IsLast()) {
      break;
    }

    fst->ReadNextRealArc(scratch_arc, in.get());
  }

  std::cout << "h - " << h << std::endl;
  std::cout << "====================" << std::endl; 

  return (h & std::numeric_limits<int64_t>::max());
}

}  // namespace util
}  // namespace core
}  // namespace lucene


#endif  // SRC_UTIL_FSTBUILDER_H_
