#include "core/framework/utils.h"

#include "core/graph/graph.h"

#include "core/framework/execution_providers.h"
#include "core/framework/kernel_def_builder.h"
#include "core/framework/kernel_registry_manager.h"
#include "core/framework/op_kernel.h"
#include "core/framework/session_state.h"

namespace Lotus {
namespace Helpers {

const KernelDef* GetKernelDef(const KernelRegistryManager& kernel_registry,
                              const LotusIR::Node& node) {
  const KernelCreateInfo* kernel_create_info = nullptr;
  const KernelDef* kernel_def = nullptr;

  if (kernel_registry.SearchKernelRegistry(node, &kernel_create_info).IsOK()) {
    kernel_def = kernel_create_info->kernel_def.get();
  }

  return kernel_def;
}

const KernelDef* GetKernelDef(const LotusIR::Graph& graph,
                              const KernelRegistryManager& kernel_registry,
                              const LotusIR::NodeIndex node_id) {
  auto node = graph.GetNode(node_id);
  LOTUS_ENFORCE(nullptr != node);

  return Helpers::GetKernelDef(kernel_registry, *node);
}

AllocatorPtr GetAllocator(const ExecutionProviders& exec_providers, const AllocatorInfo& allocator_info) {
  auto exec_provider = exec_providers.Get(allocator_info);
  if (exec_provider == nullptr) {
    return nullptr;
  }

  return exec_provider->GetAllocator(allocator_info.mem_type);
}

/*
AllocatorPtr GetAllocator(const SessionState& session_state, const AllocatorInfo& allocator_info) {
  return GetAllocator(session_state.GetExecutionProviders(), allocator_info);
}*/

}  // namespace Helpers
}  // namespace Lotus