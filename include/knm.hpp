#pragma once

#include <sdsl/int_vector.hpp>
#include <sdsl/int_vector_mapper.hpp>
#include "sdsl/suffix_arrays.hpp"
#include "sdsl/suffix_trees.hpp"
#include <sdsl/suffix_array_algorithm.hpp>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <string>
#include <iomanip>

#include "utils.hpp"
#include "collection.hpp"
#include "index_succinct.hpp"
#include "constants.hpp"


// Computes the probability of P( x | a b c ... ) using raw occurrence counts.
// Note that the backoff probability uses the lower order variants of this method.
//      idx -- the index
//      pattern -- iterators into pattern (is this in order or reversed order???)
//      lb, rb -- left and right bounds on the forward CST (spanning the full index for this
//      method???)
template <class t_idx, class t_pat_iter, class t_node>
double highestorder(const t_idx& idx, uint64_t level, const bool unk,
                    t_pat_iter pattern_begin, t_pat_iter pattern_end,
                    t_node &node, t_node &node_rev,
                    uint64_t& char_pos, uint64_t& d, uint64_t ngramsize)
{
    //std::cout << "highestorder level=" << level << " pattern=";
    //std::copy(pattern_begin, pattern_end, std::ostream_iterator<int>(std::cout, " "));
    //std::cout << std::endl;
    double backoff_prob = pkn(idx, level, unk, pattern_begin + 1, pattern_end, node, node_rev,
                              char_pos, d, ngramsize);
    uint64_t denominator = 0;
    uint64_t c = 0;

    if (forward_search(idx.m_cst_rev, node_rev, d, *pattern_begin, char_pos) > 0) {
        c = idx.m_cst_rev.size(node_rev);
    }
    uint64_t pattern_size = std::distance(pattern_begin, pattern_end);
    double D = 0;
    if (pattern_size == ngramsize)
        D = idx.discount(ngramsize);
    else
        // which is the special case of n<ngramsize that starts with <s>
        D = idx.discount(pattern_size, true);
    double numerator = 0;
    if (!unk && c - D > 0) {
        numerator = c - D;
    }

    uint64_t lb = idx.m_cst.lb(node), rb = idx.m_cst.rb(node);
    if (backward_search(idx.m_cst.csa, lb, rb, *pattern_begin, lb, rb) > 0) {
        node = idx.m_cst.node(lb, rb);
        denominator = idx.m_cst.size(node);
        double N1plus_front = idx.N1PlusFront(node, pattern_begin, pattern_end - 1);
        double output = (numerator / denominator) + (D * N1plus_front / denominator) * backoff_prob;
        return output;
    } else {
        return backoff_prob;
    }

}

template <class t_idx, class t_pat_iter, class t_node>
double lowerorder(const t_idx& idx, uint64_t level, const bool unk,
                    t_pat_iter pattern_begin, t_pat_iter pattern_end,
                    t_node &node, t_node &node_rev,
                   uint64_t& char_pos, uint64_t& d,
                  uint64_t ngramsize)
{
    //std::cout << "lowerorder level=" << level << " pattern=";
    //std::copy(pattern_begin, pattern_end, std::ostream_iterator<int>(std::cout, " "));
    //std::cout << std::endl;

    level = level - 1;
    double backoff_prob = pkn(idx, level, unk, pattern_begin + 1, pattern_end, node, node_rev, char_pos, d, ngramsize);

    uint64_t c = 0;
    if (forward_search(idx.m_cst_rev, node_rev, d, *pattern_begin, char_pos) > 0) {
        c = idx.N1PlusBack(node_rev, pattern_begin, pattern_end);
    }

    double D = idx.discount(level, true);
    double numerator = 0;
    if (!unk && c - D > 0) {
        numerator = c - D;
    }

    uint64_t lb = idx.m_cst.lb(node), rb = idx.m_cst.rb(node);
    if (backward_search(idx.m_cst.csa, lb, rb, *pattern_begin, lb, rb) > 0) { 
        node = idx.m_cst.node(lb, rb);
        auto N1plus_front = idx.N1PlusFront(node, pattern_begin, pattern_end - 1);
        auto back_N1plus_front = idx.N1PlusFrontBack(node, pattern_begin, pattern_end - 1);
        // FIXME: for the index_succinct version of N1PlusFrontBack this call above can be
        // avoided for patterns that begin with <s> and/or end with </s> using 'N1PlusFront' and 'c'
        // But might not be worth bothering, as these counts are stored explictly for the
        // faster version of the code so there would be no win here.
        d++;
        //std::cout << "N1PLUSFRONTBACK = " << back_N1plus_front << "\n";
        return (numerator / back_N1plus_front)
               + (D * N1plus_front / back_N1plus_front) * backoff_prob;
    } else {
        // TODO CHECK: what happens to the bounds when this is false?
        node = idx.m_cst.node(lb, rb);
        return backoff_prob;
    }
}

template <class t_idx, class t_pat_iter, class t_node>
double lowestorder(const t_idx& idx, 
                    t_pat_iter pattern_begin, t_pat_iter pattern_end,
                    t_node &node_rev, uint64_t& char_pos, uint64_t& d)
{
    //std::cout << "lowestorder pattern=";
    //std::copy(pattern_begin, pattern_end, std::ostream_iterator<int>(std::cout, " "));
    //std::cout << std::endl;

    double denominator = 0;
    forward_search(idx.m_cst_rev, node_rev, d, *pattern_begin, char_pos);
    d++;
    denominator = idx.m_precomputed.N1plus_dotdot;
    int numerator = idx.N1PlusBack(node_rev, pattern_begin, pattern_end); // TODO precompute this
    double probability = (double)numerator / denominator;
    return probability;
}

// special lowest order handler for P_{KN}(unknown)
template <class t_idx> 
double lowestorder_unk(const t_idx& idx)
{
    double denominator = idx.m_precomputed.N1plus_dotdot;
    double probability = idx.discount(1, true) / denominator;
    return probability;
}

template <class t_idx, class t_pat_iter, class t_node>
double pkn(const t_idx& idx, uint64_t level, const bool unk,
           t_pat_iter pattern_begin, t_pat_iter pattern_end,
           t_node &node, t_node &node_rev,
           uint64_t& char_pos, uint64_t& d, uint64_t ngramsize)
{
    uint64_t size = std::distance(pattern_begin, pattern_end);
    double probability = 0;
    if ((size == ngramsize && ngramsize != 1) || (*pattern_begin == PAT_START_SYM)) {
        probability = highestorder(idx, level, unk, pattern_begin, pattern_end, node, node_rev, char_pos, d, ngramsize);
    } else if (size < ngramsize && size != 1) {
        assert(size > 0);
        probability = lowerorder(idx, level, unk, pattern_begin, pattern_end, node, node_rev, char_pos, d, ngramsize);
    } else if (size == 1 || ngramsize == 1) {
        if (!unk) {
            probability = lowestorder(idx, pattern_end - 1, pattern_end, node_rev, char_pos, d);
        } else {
            probability = lowestorder_unk(idx);
        }
    }
    return probability;
}

template <class t_idx, class t_pattern>
double run_query_knm(const t_idx& idx, const t_pattern& word_vec, uint64_t& M, uint64_t ngramsize)
{
    double final_score = 0;
    std::deque<uint64_t> pattern_deq;
    for (const auto& word : word_vec) {
        pattern_deq.push_back(word);
        if (word == PAT_START_SYM)
            continue;
        if (pattern_deq.size() > ngramsize) {
            pattern_deq.pop_front();
        }
        std::vector<uint64_t> pattern(pattern_deq.begin(), pattern_deq.end());
        typedef typename t_idx::cst_type::node_type t_node;
        t_node node = idx.m_cst.root();
        t_node node_rev = idx.m_cst_rev.root();
        uint64_t char_pos = 0, d = 0;
        int size = std::distance(pattern.begin(), pattern.end());
        bool unk = false;
        if (pattern.back() == 77777) { // what the heck, we have an UNK_SYM sentinel, why not use it?
            unk = true;
            M = M - 1; // excluding OOV from perplexity - identical to SRILM ppl
        }
        double score = pkn(idx, size, unk, pattern.begin(), pattern.end(), 
                node, node_rev, char_pos, d, ngramsize);
        final_score += log10(score);
    }
    return final_score;
}

template <class t_idx, class t_pattern>
double gate(const t_idx& idx, t_pattern &pattern, uint32_t ngramsize)
{
    auto pattern_size = pattern.size();
    pattern.push_back(PAT_END_SYM);
    pattern.insert(pattern.begin(), PAT_START_SYM);
    // run the query
    uint64_t M = pattern_size + 1;
    double sentenceprob = run_query_knm(idx, pattern, M, ngramsize);
    double perplexity = pow(10, -(1 / (double)M) * sentenceprob);
    return perplexity;
}


#if 0
template <class t_idx, class t_pat_iter>
double conditional_probability(const t_idx& idx, 
        t_pat_iter pattern_begin,
        t_pat_iter pattern_end)
{
    typedef typename t_idx::cst_type::node_type t_node;
    int ngramsize = idx.m_precomputed.max_ngram_len;
    double probability = 1.0;
    t_node node_fwd = idx.m_cst.root();
    t_node node_rev = idx.m_cst_rev.root();
    size_t size = std::distance(pattern_begin, pattern_end);
    uint64_t d = 0; // ???

    for (unsigned i = 1; i <= size; ++i) {
        t_pat_iter pat_start = pattern_end - i;

        if (i == 1) {
            double denominator = 0;
            forward_search(idx.m_cst_rev, node_rev, d, *start, 0);
            d++;
            denominator = idx.m_precomputed.N1plus_dotdot;
            int numerator = idx.N1PlusBack(node_rev, pat_start, pattern_end); 
            probability = (double)numerator / denominator;
        } else if (i < size) {
        }
    }
}
#endif
