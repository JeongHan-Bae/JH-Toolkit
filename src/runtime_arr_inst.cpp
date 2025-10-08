/**
 * @file runtime_arr_inst.cpp
 * @brief Compilation unit for <code>jh::runtime_arr&lt;bool&gt;</code> and its conjugate form.
 *
 * <p>
 * This file includes the specialization of
 * <code>jh::runtime_arr&lt;bool&gt;</code> (bit-packed) and explicitly instantiates
 * its byte-based conjugate form:
 * <code>jh::runtime_arr&lt;bool, jh::runtime_arr_helper::bool_flat_alloc&gt;</code>.
 * </p>
 *
 * <h4>Purpose</h4>
 * <p>
 * The bit-packed specialization (<code>runtime_arr&lt;bool&gt;</code>)
 * is defined as a concrete class type rather than a template instance.
 * Including this file ensures that both bit-packed and byte-based
 * boolean array implementations are compiled into the library, providing
 * consistent linkage and ready-to-use symbols for both variants.
 * </p>
 *
 * <h4>Conjugate Pair</h4>
 * <table>
 *   <tr><th>Variant</th><th>Definition</th><th>Storage Layout</th></tr>
 *   <tr>
 *     <td><b>Bit-packed</b></td>
 *     <td><code>jh::runtime_arr&lt;bool&gt;</code></td>
 *     <td>1 bit per element (<code>uint64_t[]</code> backing)</td>
 *   </tr>
 *   <tr>
 *     <td><b>Byte-based</b></td>
 *     <td><code>jh::runtime_arr&lt;bool, jh::runtime_arr_helper::bool_flat_alloc&gt;</code></td>
 *     <td>1 byte per element (<code>uint8_t[]</code> backing)</td>
 *   </tr>
 * </table>
 *
 * <p>
 * Both variants share identical APIs and semantics; they differ only
 * in storage representation and allocator policy.
 * </p>
 *
 * @see jh::runtime_arr
 * @see jh::runtime_arr_helper::bool_flat_alloc
 */
#include "jh/runtime_arr.h"

// implicitly force static compilation of both conjugate forms
namespace jh {
    template class runtime_arr<bool, runtime_arr_helper::bool_flat_alloc>;
}
