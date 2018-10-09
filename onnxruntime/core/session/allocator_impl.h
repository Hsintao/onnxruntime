// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once
#include "core/session/allocator.h"
#include "core/framework/allocator.h"

namespace onnxruntime {
class AllocatorWrapper : public IAllocator {
 public:
  AllocatorWrapper(ONNXRuntimeAllocator* impl) : impl_(impl) {
    (*impl)->AddRef(impl);
  }
  ~AllocatorWrapper() {
    (*impl_)->Release(impl_);
  }
  void* Alloc(size_t size) override {
    return (*impl_)->Alloc(impl_, size);
  }
  void Free(void* p) override {
    return (*impl_)->Free(impl_, p);
  }
  const AllocatorInfo& Info() const override {
    return *(AllocatorInfo*)(*impl_)->Info(impl_);
  }

 private:
  ONNXRuntimeAllocator* impl_;
};
}  // namespace onnxruntime
