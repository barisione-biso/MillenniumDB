#include "tuple_buffer_block.h"

#include <cassert>

#include "storage/buffer_manager.h"

using namespace std;

TupleBufferBlock::TupleBufferBlock(FileId file_id,
                                   uint_fast32_t page_number,
                                   const std::vector<VarId>& enumeration_vars) :
    page             (buffer_manager.get_page(file_id, page_number)),
    tuple_size       (enumeration_vars.size()),
    enumeration_vars (enumeration_vars),
    max_tuples       (get_max_tuples(tuple_size)),
    tuple_count      (reinterpret_cast<uint32_t*>(page.get_bytes())),
    tuples           (reinterpret_cast<uint64_t*>(page.get_bytes() + sizeof(uint32_t)))
    { }


TupleBufferBlock::~TupleBufferBlock() {
    buffer_manager.unpin(page);
}


void TupleBufferBlock::append_tuple(const std::vector<ObjectId>& tuple) {
    for (std::size_t i = 0; i < tuple.size(); ++i) {
        tuples[ (*tuple_count) * tuple_size + i ] = tuple[i].id;
    }
    ++(*tuple_count);
    page.make_dirty(); // TODO: may be better to mark dirty in destructor?
}


void TupleBufferBlock::assign_to_binding(BindingId& binding, uint_fast32_t pos_in_block) {
    for (uint_fast32_t i = 0; i < enumeration_vars.size(); i++) {
        binding.add(enumeration_vars[i],
                    ObjectId(tuples[tuple_size*pos_in_block + i])
        );
    }
}
