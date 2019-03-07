// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once
#include <ctime>
#include "core/common/logging/logging.h"
#include "core/framework/op_kernel.h"
#include "NvInfer.h"
#include "NvOnnxParser.h"

namespace onnxruntime {

static const int kMaxBatchSize = 16;
static const int kMaxWorkSpaceSize = 16 << 20;

#define CHECK_CUDA(call)                         \
  do {                                           \
    cudaError_t status = call;                   \
    if(status != cudaSuccess) {                  \
      return -1;                                 \
    }                                            \
  } while(0)

struct InferDeleter {
  template <typename T>
  void operator()(T* obj) const {
    if (obj) {
      obj->destroy();
    }
  }
};

template <typename T>
using unique_pointer = std::unique_ptr<T, InferDeleter>;

class TensorrtLogger : public nvinfer1::ILogger {
    nvinfer1::ILogger::Severity verbosity_;
public:
    TensorrtLogger(Severity verbosity=Severity::kWARNING)
        : verbosity_(verbosity) {}
    void log(Severity severity, const char* msg) override {
        if( severity <= verbosity_ ) {
            time_t rawtime = std::time(0);
            char buf[256];
            strftime(&buf[0], 256,
                     "%Y-%m-%d %H:%M:%S",
                     std::gmtime(&rawtime));
            const char* sevstr = (severity == Severity::kINTERNAL_ERROR ? "    BUG" :
                                  severity == Severity::kERROR          ? "  ERROR" :
                                  severity == Severity::kWARNING        ? "WARNING" :
                                  severity == Severity::kINFO           ? "   INFO" :
                                  "UNKNOWN");
            LOGS_DEFAULT(WARNING) << "[" << buf << " " << sevstr << "] " << msg;
        }
    }
};

// Information needed to construct trt execution providers.
struct TensorrtExecutionProviderInfo {
  int device_id{0};
};

// Information to construct kernel function state.
struct TensorrtFuncState {
  AllocateFunc test_allocate_func = nullptr;
  DestroyFunc test_release_func = nullptr;
  AllocatorHandle allocator = nullptr;
  nvonnxparser::IParser* parser = nullptr;
  nvinfer1::ICudaEngine* engine = nullptr;
  nvinfer1::IExecutionContext* context = nullptr;
  std::vector<std::vector<int>> input_info;
  std::vector<std::vector<int>> output_info;
  std::vector<std::vector<int64_t>> output_shapes;
};

// Logical device representation.
class TensorrtExecutionProvider : public IExecutionProvider {
 public:
  TensorrtExecutionProvider();
  virtual ~TensorrtExecutionProvider();

  std::vector<std::unique_ptr<ComputeCapability>>
  GetCapability(const onnxruntime::GraphViewer& graph,
                const std::vector<const KernelRegistry*>& /*kernel_registries*/) const override;

  common::Status Compile(const std::vector<onnxruntime::Node*>& fused_nodes,
                         std::vector<NodeComputeInfo>& node_compute_funcs) override;

  Status CopyTensor(const Tensor& src, Tensor& dst) const override;

  const void* GetExecutionHandle() const noexcept override {
    return nullptr;
  }

  std::shared_ptr<KernelRegistry> GetKernelRegistry() const override;

 private:
  int device_id_;
  std::unordered_map<std::string, unique_pointer<nvonnxparser::IParser>> parsers_;
  std::unordered_map<std::string, unique_pointer<nvinfer1::ICudaEngine>> engines_;
  std::unordered_map<std::string, unique_pointer<nvinfer1::IExecutionContext>> contexts_;
  std::unordered_map<std::string, std::vector<std::vector<int>>> input_info_;
  std::unordered_map<std::string, std::vector<std::vector<int>>> output_info_;
  std::unordered_map<std::string, std::vector<std::vector<int64_t>>> output_shapes_;
};

}  // namespace onnxruntime
