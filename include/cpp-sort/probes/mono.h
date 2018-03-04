/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Morwenn
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef CPPSORT_PROBES_MONO_H_
#define CPPSORT_PROBES_MONO_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <functional>
#include <iterator>
#include <type_traits>
#include <cpp-sort/sorter_facade.h>
#include <cpp-sort/sorter_traits.h>
#include <cpp-sort/utility/as_function.h>
#include <cpp-sort/utility/functional.h>
#include "../detail/iterator_traits.h"

namespace cppsort::probe
{
    namespace detail
    {
        struct mono_impl
        {
            template<
                typename ForwardIterator,
                typename Compare = std::less<>,
                typename Projection = utility::identity,
                typename = std::enable_if_t<
                    is_projection_iterator_v<Projection, ForwardIterator, Compare>
                >
            >
            auto operator()(ForwardIterator first, ForwardIterator last,
                            Compare compare={}, Projection projection={}) const
                -> cppsort::detail::difference_type_t<ForwardIterator>
            {
                using difference_type = cppsort::detail::difference_type_t<ForwardIterator>;
                auto&& comp = utility::as_function(compare);
                auto&& proj = utility::as_function(projection);

                if (first == last || std::next(first) == last) {
                    return 0;
                }

                auto current = first;
                auto next = std::next(first);

                // The result of mono is the number of ascending runs founds +
                // the number of descending runs founds minus one (that minus
                // one is needed to make mono a proper measure of presortedness,
                // returning 0 when the collection is sorted), so the whole
                // algorithm is designed in a way that it will be "late by one"
                // on the count of runs
                difference_type count = 0;

                while (next != last) {

                    if (compare(proj(*current), proj(*next))) {
                        // Look for an ascending run
                        do {
                            current = next;
                            ++next;
                            if (next == last) {
                                return count;
                            }
                        } while(not compare(proj(*next), proj(*current)));
                        ++count;

                    } else if (compare(proj(*next), proj(*current))) {
                        // Look for a descending run
                        do {
                            current = next;
                            ++next;
                            if (next == last) {
                                return count;
                            }
                        } while(not compare(proj(*current), proj(*next)));
                        ++count;
                    }

                    //
                    // When we have reached the end of an ascending or descending run,
                    // we need to increase the current and next iterators in order to
                    // reach the next pair to compute
                    //
                    // We also try to avoid being biased towards either ascending or
                    // descending runs: when two values compare equivalent, increment the
                    // iterators and wait until two elements compare unequal or until the
                    // end of the collection has been reached
                    //
                    current = next;
                    ++next;
                }
                return count;
            }
        };
    }

    inline constexpr sorter_facade<detail::mono_impl> mono{};
}

#endif // CPPSORT_PROBES_MONO_H_
