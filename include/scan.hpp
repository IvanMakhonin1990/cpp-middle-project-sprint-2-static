#pragma once

#include "format_string.hpp"
#include "parse.hpp"
#include "types.hpp"
#include <tuple>

namespace stdx {

// ============================================================================
// Главная функция scan
// ============================================================================
template <details::fixed_string fmt_str, details::fixed_string src_str, typename... Ts>
consteval details::scan_result<Ts...> scan() {
    constexpr auto fmt_info = details::get_number_placeholders<fmt_str>();
    static_assert(fmt_info.has_value(), "Format string is invalid");
    static_assert(sizeof...(Ts) == fmt_info.value(), "Mismatch between types and placeholders");

    if constexpr (sizeof...(Ts) == 0) {
        return details::scan_result<Ts...>();
    } else {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return details::scan_result<Ts...>(
                details::parse_input<I, fmt_str, src_str, std::tuple_element_t<I, std::tuple<Ts...>>>()...);
        }(std::index_sequence_for<Ts...>{});
    }
}
}  // namespace stdx
