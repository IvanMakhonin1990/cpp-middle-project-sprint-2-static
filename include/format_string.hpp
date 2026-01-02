#pragma once

#include <array>
#include <expected>
#include <string_view>

#include "types.hpp"

namespace stdx::details {

// ============================================================================
// format_string: Шаблонный класс для работы с форматирующей строкой
// ============================================================================

// Получение количества плейсхолдеров и проверка корректности
template <fixed_string str>
consteval std::expected<std::size_t, parse_error> get_number_placeholders() {
    constexpr std::string_view sv(str.data, str.size());

    if (sv.empty())
        return 0;

    std::size_t placeholder_count = 0;
    std::size_t pos = 0;

    while (pos < sv.size()) {
        // Пропускаем все символы до '{'
        if (sv[pos] != '{') {
            ++pos;
            continue;
        }

        // Проверяем незакрытый плейсхолдер
        if (pos + 1 >= sv.size()) {
            return std::unexpected(parse_error{"Unclosed last placeholder"});
        }

        // Начало плейсхолдера
        ++placeholder_count;
        ++pos;

        // Проверка спецификатора формата
        if (sv[pos] == '%') {
            ++pos;
            if (pos >= sv.size()) {
                return std::unexpected(parse_error{"Unclosed last placeholder"});
            }

            // Проверяем допустимые спецификаторы
            const char spec = sv[pos];
            constexpr std::string_view valid_specs = "dufs";

            if (valid_specs.find(spec) == std::string_view::npos) {
                return std::unexpected(parse_error{"Invalid specifier."});
            }
            ++pos;
        }

        // Проверяем закрывающую скобку
        if (pos >= sv.size() || sv[pos] != '}') {
            return std::unexpected(parse_error{"'}' hasn't been found in appropriate place"});
        }
        ++pos;
    }

    return placeholder_count;
}

// Получение позиций плейсхолдеров
template <fixed_string str, std::size_t Count>
consteval std::array<std::pair<std::size_t, std::size_t>, Count> get_placeholder_positions() {
    constexpr std::string_view sv(str.data, str.size());
    std::array<std::pair<std::size_t, std::size_t>, Count> result{};

    std::size_t placeholder_count = 0;
    std::size_t pos = 0;

    while (pos < sv.size() && placeholder_count < Count) {
        // Ищем '{'
        if (sv[pos] != '{') {
            ++pos;
            continue;
        }

        std::size_t start = pos;
        ++pos;

        // Пропускаем спецификатор если есть
        if (pos < sv.size() && sv[pos] == '%') {
            ++pos;
            if (pos < sv.size())
                ++pos;  // Пропускаем сам спецификатор
        }

        // Ищем '}'
        std::size_t end = pos;
        if (pos < sv.size() && sv[pos] == '}') {
            end = pos;
        }

        result[placeholder_count] = {start, end};
        ++placeholder_count;
        ++pos;
    }

    return result;
}

// Основной класс format_string
template <fixed_string fmt_str>
class format_string {
public:
    static constexpr fixed_string fmt = fmt_str;

    static constexpr auto number_placeholders_result = get_number_placeholders<fmt_str>();
    static_assert(number_placeholders_result.has_value(), "Format string validation failed at compile time");
    static constexpr std::size_t number_placeholders = number_placeholders_result.value();

    static constexpr auto placeholder_positions = get_placeholder_positions<fmt_str, number_placeholders>();

    // Получить спецификатор формата для i-го плейсхолдера
    static constexpr char get_format_spec(std::size_t i) {
        constexpr std::string_view sv(fmt.data, fmt.size());

        if (i >= number_placeholders) {
            return '\0';
        }

        auto [start, end] = placeholder_positions[i];
        // Ищем '%' после '{'
        if (start + 1 < sv.size() && sv[start + 1] == '%') {
            if (start + 2 < sv.size()) {
                return sv[start + 2];
            }
        }

        // По умолчанию для строк
        return 's';
    }
};

}  // namespace stdx::details
