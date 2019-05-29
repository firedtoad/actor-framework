/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright 2011-2018 Dominik Charousset                                     *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#pragma once

#include <vector>

#include "caf/actor_system.hpp"
#include "caf/execution_unit.hpp"
#include "caf/exit_reason.hpp"
#include "caf/local_actor.hpp"
#include "caf/resumable.hpp"
#include "caf/scheduled_actor.hpp"

namespace caf {
namespace detail {

struct cleanup_and_release {
  cleanup_and_release(actor_system& sys) : sys_(sys) {
    // nop
  }
  void operator()(resumable* ptr) {
    class dummy_unit : public execution_unit {
    public:
      dummy_unit(actor_system* sys) : execution_unit(sys) {
        // nop
      }
      void exec_later(resumable* job) override {
        resumables.push_back(job);
      }
      std::vector<resumable*> resumables;
    };
    switch (ptr->subtype()) {
      case resumable::scheduled_actor:
      case resumable::io_actor: {
        auto dptr = static_cast<scheduled_actor*>(ptr);
        dummy_unit dummy{&sys_};
        dptr->cleanup(make_error(exit_reason::user_shutdown), &dummy);
        while (!dummy.resumables.empty()) {
          auto sub = dummy.resumables.back();
          dummy.resumables.pop_back();
          switch (sub->subtype()) {
            case resumable::scheduled_actor:
            case resumable::io_actor: {
              auto dsub = static_cast<scheduled_actor*>(sub);
              dsub->cleanup(make_error(exit_reason::user_shutdown), &dummy);
              break;
            }
            default:
              break;
          }
        }
        break;
      }
      case resumable::function_object: {
        dummy_unit dummy{&sys_};
        ptr->resume(&dummy, std::numeric_limits<size_t>::max());
        break;
      }
      default:
        break;
    }
    intrusive_ptr_release(ptr);
  }

  actor_system& sys_;
};
} // namespace detail
} // namespace caf
