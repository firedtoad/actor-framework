#include "caf/test/bdd_dsl.hpp"
#include "caf/test/io_dsl.hpp"

using calculator
  = caf::typed_actor<caf::result<int32_t>(caf::add_atom, int32_t, int32_t),
                     caf::result<int32_t>(caf::sub_atom, int32_t, int32_t)>;

CAF_BEGIN_TYPE_ID_BLOCK(io_test, caf::first_custom_type_id)

  CAF_ADD_TYPE_ID(io_test, (calculator))

CAF_END_TYPE_ID_BLOCK(io_test)
