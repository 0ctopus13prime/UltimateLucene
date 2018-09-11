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


#include <Util/Pack/BulkOperation.h>

using lucene::core::util::BulkOperation;
using lucene::core::util::BulkOperationPackedSingleBlock;

/**
 *  BulkOperation
 */
BulkOperation* BulkOperation::packed_bulk_ops[] = {
  new BulkOperationPacked1(),
  new BulkOperationPacked2(),
  new BulkOperationPacked3(),
  new BulkOperationPacked4(),
  new BulkOperationPacked5(),
  new BulkOperationPacked6(),
  new BulkOperationPacked7(),
  new BulkOperationPacked8(),
  new BulkOperationPacked9(),
  new BulkOperationPacked10(),
  new BulkOperationPacked11(),
  new BulkOperationPacked12(),
  new BulkOperationPacked13(),
  new BulkOperationPacked14(),
  new BulkOperationPacked15(),
  new BulkOperationPacked16(),
  new BulkOperationPacked17(),
  new BulkOperationPacked18(),
  new BulkOperationPacked19(),
  new BulkOperationPacked20(),
  new BulkOperationPacked21(),
  new BulkOperationPacked22(),
  new BulkOperationPacked23(),
  new BulkOperationPacked24(),
  new BulkOperationPacked(25), 
  new BulkOperationPacked(26), 
  new BulkOperationPacked(27), 
  new BulkOperationPacked(28), 
  new BulkOperationPacked(29), 
  new BulkOperationPacked(30), 
  new BulkOperationPacked(31), 
  new BulkOperationPacked(32), 
  new BulkOperationPacked(33), 
  new BulkOperationPacked(34), 
  new BulkOperationPacked(35), 
  new BulkOperationPacked(36), 
  new BulkOperationPacked(37), 
  new BulkOperationPacked(38), 
  new BulkOperationPacked(39), 
  new BulkOperationPacked(40), 
  new BulkOperationPacked(41), 
  new BulkOperationPacked(42), 
  new BulkOperationPacked(43), 
  new BulkOperationPacked(44), 
  new BulkOperationPacked(45), 
  new BulkOperationPacked(46), 
  new BulkOperationPacked(47), 
  new BulkOperationPacked(48), 
  new BulkOperationPacked(49), 
  new BulkOperationPacked(50), 
  new BulkOperationPacked(51), 
  new BulkOperationPacked(52), 
  new BulkOperationPacked(53), 
  new BulkOperationPacked(54), 
  new BulkOperationPacked(55), 
  new BulkOperationPacked(56), 
  new BulkOperationPacked(57), 
  new BulkOperationPacked(58), 
  new BulkOperationPacked(59), 
  new BulkOperationPacked(60), 
  new BulkOperationPacked(61), 
  new BulkOperationPacked(62), 
  new BulkOperationPacked(63), 
  new BulkOperationPacked(64)
};

BulkOperation* BulkOperation::packed_single_block_bulk_ops[] = {
  new BulkOperationPackedSingleBlock(1),
  new BulkOperationPackedSingleBlock(2),
  new BulkOperationPackedSingleBlock(3),
  new BulkOperationPackedSingleBlock(4),
  new BulkOperationPackedSingleBlock(5),
  new BulkOperationPackedSingleBlock(6),
  new BulkOperationPackedSingleBlock(7),
  new BulkOperationPackedSingleBlock(8),
  new BulkOperationPackedSingleBlock(9),
  new BulkOperationPackedSingleBlock(10),
  nullptr,
  new BulkOperationPackedSingleBlock(12),
  nullptr,
  nullptr,
  nullptr,
  new BulkOperationPackedSingleBlock(16),
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  new BulkOperationPackedSingleBlock(21),
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  new BulkOperationPackedSingleBlock(32)
};
