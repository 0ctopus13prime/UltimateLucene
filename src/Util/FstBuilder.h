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
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

namespace lucene {
namespace core {
namespace util {

template <typename T>
class Builder;

template<typename T>
class NodeHash {
 private:
  PagedGrowableWriter table;
  uint64_t count;
  uint64_t mask;
  FST<T>* fst;
  typename FST<T>::Arc scratch_arc;
  std::unique_ptr<FSTBytesReader> in;

 private:
  // Only for rehash
  NodeHash(PagedGrowableWriter&& table, NodeHash<T>&& other)
    : table(std::forward<PagedGrowableWriter>(table)),
      count(other.count),
      mask(this->table.Size() - 1),
      fst(other.fst),
      scratch_arc(std::move(other.scratch_arc)),
      in(std::move(other.in)) {
  }

  bool NodesEqual(typename Builder<T>::UnCompiledNode& node,
                  const uint64_t address) {
    fst->ReadFirstRealTargetArc(address, scratch_arc, in.get());
    if (scratch_arc.bytes_per_arc != 0 &&
        node.num_arcs != scratch_arc.num_arcs) {
      return false; 
    }

    for (uint32_t arc_upto = 0 ; arc_upto < node.num_arcs ; ++arc_upto) {
      typename Builder<T>::Arc& arc = node.arcs[arc_upto];
      if (arc.label != scratch_arc.label ||
          arc.output != scratch_arc.output ||
          dynamic_cast<typename Builder<T>::CompiledNode*>(arc.target)->node
            != scratch_arc.target ||
          arc.next_final_output != scratch_arc.next_final_output ||
          arc.is_final != scratch_arc.IsFinal()) {
        return false;
      } else if (scratch_arc.IsLast()) {
        return (arc_upto == (node.num_arcs - 1));
      }

      fst->ReadNextRealArc(scratch_arc, in.get());
    }

    return false;
  }

  int64_t Hash(typename Builder<T>::UnCompiledNode& node) {
    const uint32_t PRIME = 31;  
    int64_t h = 0;

    for (uint32_t arc_idx = 0 ; arc_idx < node.num_arcs ; ++arc_idx) {
      typename Builder<T>::Arc& arc = node.arcs[arc_idx];
      h = PRIME * h + arc.label;
      const int64_t n =
        dynamic_cast<typename Builder<T>::CompiledNode*>(arc.target)->node;
      h = PRIME * h + static_cast<int32_t>(n ^ (n >> 32));
      h = PRIME * h + arc.output.HashCode();
      h = PRIME * h + arc.next_final_output.HashCode();
      if (arc.is_final) {
        h += 17;
      }
    }

    return (h & std::numeric_limits<int64_t>::max());
  }

  int64_t Hash(const int64_t node) {
    const uint32_t PRIME = 31;
    int64_t h = 0;
    fst->ReadFirstRealTargetArc(node, scratch_arc, in.get());
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

 public:
  NodeHash(FST<T>* fst,
           std::unique_ptr<FSTBytesReader>&& in)
    : table(16, 1 << 27, 8, PackedInts::COMPACT),
      count(0),
      mask(15),
      fst(fst),
      scratch_arc(),
      in(std::forward<std::unique_ptr<FSTBytesReader>>(in)) {
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

  int64_t Add(Builder<T>& builder,
              typename Builder<T>::UnCompiledNode& node_in) {
    const int64_t h = Hash(node_in);
    int64_t pos = (h & mask);
    int32_t c = 0;

    while (true) {
      const int64_t v = table.Get(pos);
      if (v == 0) {
        // Freeze & Add
        const int64_t node = builder.AddNode(node_in);
        assert(Hash(node) == h);
        count++;
        table.Set(pos, node);
        if (count > (2 * table.Size()) / 3) {
          Rehash();
        }
        return node;
      } else if (NodesEqual(node_in, v)) {
        return v;
      }

      pos = ((pos + (++c)) & mask);
    }
  }
};  // NodeHash

template<typename T>
class Builder {
 friend class NodeHash<T>;

 public:
  class Node;
  class UnCompiledNode;
  class CompiledNode;
  class Arc;

 private:
  std::unique_ptr<NodeHash<T>> dedup_hash;
  FST<T> fst;
  uint32_t min_suffix_count1;
  uint32_t min_suffix_count2;
  BytesStore& bytes;
  std::vector<uint32_t> reused_byte_per_arc;
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

 private:
  int64_t AddNode(typename Builder<T>::UnCompiledNode* node_in) {
    // TODO(0ctopus13prime): IT
    return 0L;
  }

  bool ShouldExpand(typename Builder<T>::UnCompiledNode* node) {
    // TODO(0ctopus13prime): IT
    return true;
  }

  CompiledNode* CompileNode(UnCompiledNode* node_in,
                            const uint32_t tail_length) {
    int64_t node;
    const uint64_t bytes_pos_start = bytes.GetPosition();

    if (dedup_hash &&
        (do_share_non_singleton_nodes || node_in->num_arcs <= 1) &&
        tail_length <= share_max_tail_length) {
      if (node_in->num_arcs == 0) {
        node = AddNode(node_in); 
        last_frozen_node = node;
      } else {
        node = dedup_hash->Add(*this, node_in);
      }
    } else {
      node = AddNode(node_in);
    }

    const uint64_t bytes_pos_end = bytes.GetPosition();
    if (bytes_pos_end != bytes_pos_start) {
      last_frozen_node = node;
    }

    node_in->Clear();

    return CompiledNode::New(node); 
  }
  
  void FreezeTail(const uint32_t prefix_len_plus1) {
    const uint32_t down_to = std::max(1U, prefix_len_plus1);

    for (uint32_t idx = last_input.Length() ; idx >= down_to ; --idx) {
      bool do_prune = false;
      bool do_compile = false;

      UnCompiledNode* node = frontier[idx];
      UnCompiledNode* parent = frontier[idx - 1]; 

      if (node->input_count < min_suffix_count1) {
        do_prune = true;
        do_compile = true;
      } else if(idx > prefix_len_plus1) {
        if (parent->input_count < min_suffix_count2 ||
            (min_suffix_count2 == 1 && parent->input_count == 1 && idx > 1)) {
          do_prune = true;
        } else {
          do_prune = false;
        }
        do_compile = true;
      } else {
        do_compile = (min_suffix_count2 == 0);
      }

      if (node->input_count < min_suffix_count2 ||
          (min_suffix_count2 == 1 && node->input_count == 1 && idx > 1)) {
        for (uint32_t arc_idx = 0 ; arc_idx < node->num_arcs ; ++arc_idx) {
            dynamic_cast<UnCompiledNode*>(node->arcs[arc_idx].target)->Clear(); 
        }

        node->num_arcs = 0;
      }

      if (do_prune) {
        node->Clear();
        parent->DeleteLast(last_input[idx - 1], node);
      } else {
        if (min_suffix_count2 != 0) {
          CompileAllTargets(node, last_input.Length() - idx);
        }
        T& next_final_output = node->output;
        const bool is_final = (node->is_final || node->num_arcs == 0);

        if (do_compile) {
          CompiledNode* compiled_node =
            CompileNode(node, 1 + last_input.Length() - idx); 
          parent->ReplaceLast(last_input[idx - 1],
                              compiled_node,
                              std::move(next_final_output),
                              is_final);
        } else {
          parent->ReplaceLast(last_input[idx - 1],
                              node,
                              std::move(next_final_output),
                              is_final);
          frontier[idx] = UnCompiledNode(this, idx);
        }
      }
    }
  }

  void CompileAllTargets(UnCompiledNode* node, const uint32_t tail_length) {
    for (uint32_t arc_idx = 0 ; arc_idx < node->num_arcs ; arc_idx++) {
      Arc& arc = node->arcs[arc_idx];
      if (!arc.target->IsCompiled()) {
        UnCompiledNode* n = dynamic_cast<UnCompiledNode*>(arc.target);
        if (n->num_arcs == 0) {
          arc.is_final = n->is_final = true;
        }

        arc.target = CompileNode(n, tail_length - 1);
      }
    }
  }

 public:
  Builder(const FST_INPUT_TYPE input_type,
          std::unique_ptr<Outputs<T>>&& outputs)
    : Builder(input_type,
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
  FST<T> fst;
  uint32_t min_suffix_count1;
  uint32_t min_suffix_count2;
  BytesStore& bytes;
  std::vector<uint32_t> reused_byte_per_arc;
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

  Builder(const FST_INPUT_TYPE input_type,
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
      bytes(fst.bytes),
      reused_byte_per_arc(),
      share_max_tail_length(0),
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

    reused_byte_per_arc.reserve(4);

    const uint32_t frontier_rsv_size = 20;
    frontier.reserve(frontier_rsv_size);
    for (uint32_t idx = 0 ; idx < frontier_rsv_size ; ++idx) {
      frontier.push_back(UnCompiledNode::New(this, idx));
    }
  }

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

    while (prefix_len_plus1++ < prefix_cmp_until &&
           *pos1++ != *pos2++); 

    // Minimize/Compile states from previous input's
    // orphan'd suffix
    FreezeTail(prefix_len_plus1);

  /*
    for (uint32_t idx = prefix_len_plus1 ; idx <= input.Length() ; ++idx) {
      frontier[idx - 1]->AddArc(input.ints[input.offset + idx - 1],
                                frontier[idx]);
      frontier[idx]->input_count++;
    }

    UnCompiledNode* last_node = frontier[input.length];
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
        parent_node->GetLastOutput(input.ints[input.offset + idx - 1]);

      if (!fst.outputs->IsNoOutput(last_output)) {
        // Calculate common prefix between output and last_output
        const uint32_t prefix_len = fst.outputs->PrefixLen(output, last_output);

        // Propagate last input's suffix to all arcs of node
        node.PrependOutput(
          fst.outputs->SuffixReference(last_output, prefix_len));

        // Drop suffix from last_output
        fst.outputs->DropSuffix(last_output, prefix_len);
      }

      // Drop common prefix from output
      fst.outputs->ShiftLeftSuffix(output, prefix_len);
    }

    if (last_input.Length() == input.Length() &&
        prefix_len_plus1 == (1 + input.Length())) {
      // Same input more than 1 time in a row, mapping to
      // multiple outputs
      last_node.output = fst.outputs.Merge(last_node.output, output);
    } else {
      // This new arc is private to this new input; 
      // set its arc output to the leftover output:
      frontier[prefix_len_plus1 - 1]->SetLastOutput(
        input.ints[input.Offset() + prefix_len_plus1 - 1],
        std::forward<T>(output));
    }
  */

    // Save last input
    last_input.InitInts(std::forward<IntsRef>(input));
  }

  FST<T>* Finish() {
  /*
    UnCompiledNode& root = frontier[0];
    FreezeTail(0);
    if (root.input_count < min_suffix_count1 ||
        root.input_count < min_suffix_count2 ||
        root.num_arcs == 0) {
      std::cout << "[Finish] ====================================" << std::endl;
      std::cout << "root.input_count -> " << root.input_count << std::endl;
      std::cout << "root.num_arcs -> " << root.num_arcs << std::endl;
      std::cout << "min_suffix_count1 -> " << min_suffix_count1 << std::endl;
      std::cout << "min_suffix_count2 -> " << min_suffix_count2 << std::endl;
      std::cout << "fst.AcceptEmptyOutput -> " << std::boolalpha << fst.AcceptEmptyOutput()
                << std::endl;
      std::cout << "[Finish] ====================================" << std::endl;
               
      if (!fst.AcceptEmptyOutput() ||
          (min_suffix_count1 > 0 || min_suffix_count2 > 0)) {
        return nullptr;
      }
    } else {
      if (min_suffix_count2 != 0) {
        CompileAllTargets(root, last_input.Length());
      }
    }

    fst.Finish(CompileNode(root, last_input.Length()).node);

  */
    return &fst;
  }
};  // Builder<T>

template<typename T>
class Builder<T>::Node {
 public:
  Node() = default;

  virtual ~Node() = default;

  virtual bool IsCompiled() const noexcept = 0;
};  // Builder<T>::Node

template<typename T>
class Builder<T>::Arc {
 public:
  uint32_t label;
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
};  // Builder<T>::Arc

template<typename T>
class Builder<T>::UnCompiledNode : public Builder<T>::Node {
 private:
  Outputs<T>* outputs;

 public:
  std::vector<Arc> arcs;
  uint64_t input_count;
  T output;
  uint32_t depth;
  bool is_final;

 public:
  static UnCompiledNode* New(Builder<T>* builder, const uint64_t depth) {
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
    : outputs(outputs),
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

  T& GetLastOutput(const uint32_t label_to_match) const noexcept {
    return arcs.back().output;
  }

  void AddArc(const uint32_t label, Node* target) {
    arcs.push_back(Arc());
    Arc& arc = arcs.back();
    arc.label = label;
    arc.target = target;
    outputs-MakeNoOutput(arc.output);
    outputs->MakeNoOutput(arc.next_final_output);
    arc.is_final = false;
  }

  void ReplaceLast(const uint32_t last_to_match,
                   Node* target,
                   T&& next_final_output,
                   const bool is_final) {
    Arc& arc = arcs.back();
    arc.target = target;
    arc.next_final_output = std::forward<T>(next_final_output);
    arc.is_final = is_final;
  }

  void DeleteLast(const uint32_t label, Node* target) noexcept {
    arcs.pop_back();
  }

  void SetLastOutput(const uint32_t label_to_match, T&& new_output) {
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
};  // Builder<T>::UnCompiledNode

template<typename T>
class Builder<T>::CompiledNode: public Builder<T>::Node {
 public:
  const uint64_t node;

 private:
  explicit CompiledNode(const uint64_t node)
    : node(node) {
  }

 public:
  static CompiledNode* New(Builder<T>* builder, const uint64_t node) {
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
};  // Builder<T>::CompiledNode

}  // namespace util
}  // namespace core
}  // namespace lucene


#endif  // SRC_UTIL_FSTBUILDER_H_
