#pragma once

#include <functional>
#include <memory>

using Handle = std::shared_ptr<void>;
using UniqueHandle = std::unique_ptr<void>;
using WeakHandle = std::weak_ptr<void>;
