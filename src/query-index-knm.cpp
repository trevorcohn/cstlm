#include <sdsl/int_vector.hpp>
#include <sdsl/int_vector_mapper.hpp>
#include "sdsl/suffix_arrays.hpp"
#include "sdsl/suffix_trees.hpp"
#include <sdsl/suffix_array_algorithm.hpp>
#include <iostream>
#include <math.h>
#include <algorithm>

#include "utils.hpp"
#include "collection.hpp"
#include "index_succinct.hpp"

typedef struct cmdargs {
    std::string pattern_file;
    std::string collection_dir;
    int ngramsize;
    bool ismkn;
} cmdargs_t;

void
print_usage(const char* program)
{
    fprintf(stdout, "%s -c <collection dir> -p <pattern file> -m <boolean> -n <ngramsize>\n", program);
    fprintf(stdout, "where\n");
    fprintf(stdout, "  -c <collection dir>  : the collection dir.\n");
    fprintf(stdout, "  -p <pattern file>  : the pattern file.\n");
    fprintf(stdout, "  -m <ismkn>  : the flag for Modified-KN (true), or KN (false).\n");
    fprintf(stdout, "  -n <ngramsize>  : the ngramsize integer.\n");
};

cmdargs_t
parse_args(int argc, const char* argv[])
{
    cmdargs_t args;
    int op;
    args.pattern_file = "";
    args.collection_dir = "";
    args.ismkn = false;
    while ((op = getopt(argc, (char* const*)argv, "p:c:n:m:")) != -1) {
        switch (op) {
        case 'p':
            args.pattern_file = optarg;
            break;
        case 'c':
            args.collection_dir = optarg;
            break;
        case 'm':
            if (optarg == "true")
                args.ismkn = true;
            break;
        case 'n':
            args.ngramsize = stoi((string)optarg);
            break;
        }
    }
    if (args.collection_dir == "" || args.pattern_file == "") {
        std::cerr << "Missing command line parameters.\n";
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    return args;
}

template <class t_idx>
int N1PlusFrontBack_Back(const t_idx& idx, t_idx::string_type pat, uint64_t lb, uint64_t rb)
{
    int denominator = 0;
    auto v = idx.m_cst_rev.node(lb, rb);
    int size = pat.size();
    freq = rb - lb + 1;
    if (freq == 1 && lb != rb) {
        freq = 0;
    }
    if (freq > 0) {
        if (size == idx.m_cst_rev.depth(v)) {
            auto w = idx.m_cst_rev.select_child(v, 1);
            int symbol = idx.m_cst_rev.edge(w, size + 1);
            denominator = idx.m_cst_rev.degree(v);
            if (symbol == 1) {
                denominator = denominator - 1;
            }
        } else {
            int symbol = idx.m_cst_rev.edge(v, size + 1);
            if (symbol != 1) {
                denominator = 1;
            }
        }
    } else {
    }
    return denominator;
}

template <class t_idx>
int N1PlusFrontBack_Front(const t_idx& idx, t_idx::string_type pat, uint64_t lb, uint64_t rb)
{
    int denominator = 0;
    auto v = idx.m_cst.node(lb, rb);
    freq = rb - lb + 1;
    if (freq == 1 && lb != rb) {
        freq = 0;
    }
    int size = pat.size();
    if (freq > 0) {
        if (size == idx.m_cst.depth(v)) {
            auto w = idx.m_cst.select_child(v, 1);
            int symbol = idx.m_cst.edge(w, pat_size + 1);
            int root_id = idx.m_cst.id(idx.m_cst.root());
            while (idx.m_cst.id(w) != root_id) {
                int symbol = idx.m_cst.edge(w, size + 1);
                if (symbol != 0 && symbol != 1) {
                    pat.push_back(symbol);
                    t_idx::string_type patrev = pat;
                    reverse(patrev.begin(), patrev.end());
                    uint64_t lbrev = 0, rbrev = idx.m_cst_rev.size() - 1;
                    backward_search(idx.m_cst_rev.csa, lbrev, rbrev, patrev.begin(), patrev.end(), lbrev, rbrev);
                    denominator += N1PlusFrontBack_Front(patrev, lbrev, rbrev);
                    pat.pop_back();
                }
                w = idx.m_cst.sibling(w);
            }
        } else {
            t_idx::string_type patrev = pat;
            reverse(patrev.begin(), patrev.end());
            uint64_t lbrev = 0, rbrev = idx.m_cst_rev.size() - 1;
            backward_search(idx.m_cst_rev.csa, lbrev, rbrev, pat.begin(), pat.end(), lbrev, rbrev);
            denominator += N1PlusFrontBack_Front(pat, lbrev, rbrev);
        }
    } else {
    }
    return denominator;
}

template <class t_idx>
int discount(int c)
{
    if (ismkn) {
        if (c == 1) {
            if (n1[ngramsize] != 0)
                D = D1[ngramsize];
        } else if (c == 2) {
            if (n2[ngramsize] != 0)
                D = D2[ngramsize];
        } else if (c >= 3) {
            if (n3[ngramsize] != 0)
                D = D3[ngramsize];
        }
    } else {
        D = Y[ngramsize];
    }
    return D;
}

template <class t_idx>
int N1PlusFront(node_type node, t_idx::string_type pat)
{
    int N = 0, N1 = 0, N2 = 0, N3 = 0;
    int pat_size = pat.size();
    int deg = idx.m_cst.degree(node);
    if (pat_size == idx.m_cst.depth(node)) {
        N = 0;
        if (!ismkn) {
            auto w = idx.m_cst.select_child(node, 1);
            int symbol = idx.m_cst.edge(w, pat_size + 1);
            N = deg;
            if (symbol == 1) {
                N = N - 1;
            }
        } else {
            auto w = idx.m_cst.select_child(node, 1);
            int symbol = idx.m_cst.edge(w, pat_size + 1);
            int root_id = idx.m_cst.id(idx.m_cst.root());
            while (idx.m_cst.id(w) != root_id) {
                int symbol = idx.m_cst.edge(w, pat_size + 1);
                if (symbol != 1) {
                    pat.push_back(symbol);
                    leftbound = 0, rightbound = idx.m_cst.size() - 1;
                    backward_search(idx.m_cst.csa, leftbound, rightbound, pat.begin(), pat.end(), lb, rb);
                    freq = rightbound - leftbound + 1;
                    if (freq == 1 && rightbound != leftbound) {
                        freq = 0;
                    }
                    if (freq == 1)
                        N1 += 1;
                    else if (freq == 2)
                        N2 += 1;
                    else if (freq >= 3)
                        N3 += 1;
                    pat.pop_back();
                }
                w = idx.m_cst.sibling(w);
            }
        }
    } else {
        int symbol = idx.m_cst.edge(v, pat.size() + 1);
        if (!ismkn) {
            if (symbol != 1) {
                N = 1;
            }
        }
        if (ismkn) {
            if (symbol != 1) {
                if (ismkn) {
                    pat.push_back(symbol);

                    leftbound = 0, rightbound = idx.m_cst.size() - 1;
                    backward_search(idx.m_cst.csa, leftbound, rightbound, pat.begin(), pat.end(), lb, rb);
                    freq = rightbound - leftbound + 1;
                    if (freq == 1 && rightbound != leftbound) {
                        freq = 0;
                    }
                    if (freq == 1)
                        N1 += 1;
                    else if (freq == 2)
                        N2 += 1;
                    else if (freq >= 3)
                        N3 += 1;

                    pat.pop_back();
                }
            }
        }
    }
    return N, N1, N2, N3; //TODO fix this
}

template <class t_idx>
int N1PlusBack(node_type node, t_idx::string_type patrev)
{
    int c = 0;
    int patrev_size = patrev;
    int deg = idx.m_cst_rev.degree(node);
    if (patrev_size == idx.m_cst_rev.depth(node)) {
        int ind = 0;
        c = deg;
        auto w = idx.m_cst_rev.select_child(node, 1);
        int symbol = idx.m_cst_rev.edge(w, patrev_size + 1);
        if (symbol == 1)
            c = c - 1;
    } else {
        int symbol = idx.m_cst_rev.edge(node, patrev_size + 1);
        if (symbol != 1) {
            c = 1;
        }
    }
    return c;
}

template <class t_idx>
double pkn(const t_idx& idx, t_idx::string_type pat)
{
    int size = pat.size();

    uint64_t leftbound = 0, rightbound = idx.m_cst.size() - 1;
    double probability = 0;

    if ((size == ngramsize && ngramsize != 1) || (pat[0] == STARTTAG)) { //for the highest order ngram, or the ngram that starts with <s>
        int c = 0;
        uint64_t lb = 0, rb = idx.m_cst.size() - 1;
        backward_search(idx.m_cst.csa, lb, rb, pat.begin(), pat.end(), lb, rb);
        c = rb - lb + 1;
        if (c == 1 && lb != rb) {
            c = 0;
        }
        double D = discount(c);

        double numerator = 0;
        if (c - D > 0) {
            numerator = c - D;
        }

        double denominator = 0;
        int N = 0;

        cst_sct3<csa_sada_int<> >::string_type pat2 = pat;
        pat2.erase(pat2.begin());

        pat.pop_back();
        lb = 0;
        rb = idx.m_cst.size() - 1;
        backward_search(idx.m_cst.csa, lb, rb, pat.begin(), pat.end(), lb, rb);
        freq = rb - lb + 1;
        if (freq == 1 && lb != rb) {
            freq = 0;
        }
        denominator = freq;
        if (denominator == 0) {
            cout << pat << endl;
            cout << "---- Undefined fractional number XXXZ - Backing-off ---" << endl;
            double output = pkn(idx, pat2); //TODO check this
            return output;
        }
        auto v = idx.m_cst.node(lb, rb);
        int N1 = 0, N2 = 0, N3 = 0;
        int pat_size = pat.size();
        if (freq > 0) {
            N, N1, N2, N3 = N1PlusFront(); //TODO fix this
        }
        if (ismkn) {
            double gamma = (D1[ngramsize] * N1) + (D2[ngramsize] * N2) + (D3[ngramsize] * N3);
            double output = (numerator / denominator) + (gamma / denominator) * pkn(idx, pat2);
            return output;
        } else {
            double output = (numerator / denominator) + (D * N / denominator) * pkn(idx, pat2);
            return output;
        }
    } else if (size < ngramsize && size != 1) { //for lower order ngrams

        int c = 0;
        t_idx::string_type patrev = pat;
        reverse(patrev.begin(), patrev.end());
        uint64_t lbrev = 0, rbrev = idx.m_cst_rev.size() - 1;
        backward_search(idx.m_cst_rev.csa, lbrev, rbrev, patrev.begin(), patrev.end(), lbrev, rbrev);
        freq = rbrev - lbrev + 1;
        if (freq == 1 && lbrev != rbrev) {
            freq = 0;
        }
        auto vrev = idx.m_cst_rev.node(lbrev, rbrev);
        int patrev_size = patrev.size();

        if (freq > 0) {
            c = N1PlusBack(); //TODO fix this
        }
        double D = discount(freq);

        double numerator = 0;
        if (c - D > 0) {
            numerator = c - D;
        }

        cst_sct3<csa_sada_int<> >::string_type pat3 = pat;
        pat3.erase(pat3.begin());

        pat.pop_back();

        uint64_t lb = 0, rb = idx.m_cst.size() - 1;
        backward_search(idx.m_cst.csa, lb, rb, pat.begin(), pat.end(), lb, rb);
        freq = rb - lb + 1;
        if (freq == 1 && lb != rb) {
            freq = 0;
        }
        double denominator = 0;
        int N = 0;
        if (freq != 1) {
            N, denominator = N1PlusFrontBack_Front(pat, lb, rb); // TODO return N here separatetely
        } else {
            denominator = 1;
            N = XXX; //TODO fix this
        }
        if (denominator == 0) {
            cout << "---- Undefined fractional number XXXW-backing-off---" << endl;
            double output = pkn(idx, pat3);
            return output; //TODO check
        }
        auto v = idx.m_cst.node(lb, rb);

        if (ismkn) {
            double gamma = 0;
            int N1 = 0, N2 = 0, N3 = 0;
            if (freq > 0) {
                N, N1, N2, N3 = N1PlusFront(); //TODO fix this
            }
            gamma = (D1[size] * N1) + (idx.m_D2[size] * N2) + (idx.m_D3[size] * N3);
            double output = numerator / denominator + (gamma / denominator) * pkn(idx, pat3);
            return output;
        } else {
            int pat_size = pat.size();
            double output = (numerator / denominator) + (D * N / denominator) * pkn(idx, pat3);
            return output;
        }
    } else if (size == 1 || ngramsize == 1) //for unigram
    {
        int c = 0;
        uint64_t lbrev = 0, rbrev = idx.m_cst_rev.size() - 1;
        backward_search(idx.m_cst_rev.csa, lbrev, rbrev, pat.begin(), pat.end(), lbrev, rbrev);
        freq = rbrev - lbrev + 1;
        if (freq == 1 && lbrev != rbrev)
            freq = 0;
        auto vrev = idx.m_cst_rev.node(lbrev, rbrev);
        int pat_size = pat.size();
        if (freq > 0) {
            N = N1PlusFront(); //TODO fix this
        }

        double denominator = unigramdenominator;

        if (!ismkn) {
            double output = c / denominator;
            return output;
        } else {

            double D = discount(freq);

            double numerator = 0;
            if (c - D > 0) {
                numerator = c - D;
            }

            double gamma = 0;
            int N1 = 0, N2 = 0, N3 = 0;
            N1 = n1[1];
            N2 = n2[1];
            N1 = N3plus;
            gamma = (idx.m_D1[size] * N1) + (D2[size] * N2) + (D3[size] * N3);
            double output = numerator / denominator + (gamma / denominator) * (1 / (double)idx.vocab_size());
            return output;
        }
    }
    return probability;
}

template <class t_idx>
double run_query_knm(const t_idx& idx, const std::vector<uint64_t>& word_vec)
{
    double final_score = 0;
    std::deque<uint64_t> pattern;
    for (const auto& word : word_vec) {
        pattern.push_back(word); //TODO check the pattern
        if (word == STARTTAG)
            continue;
        if (pattern_deq.size() > ngramsize) {
            pattern_deq.pop_front();
        }
        cst_sct3<csa_sada_int<> >::string_type pattern(pattern_deq.begin(), pattern_deq.end());
        double score = pkn(idx, pattern);
        final_score += log10(score);
    }
    return final_score;
}

template <class t_idx>
void run_queries(const t_idx& idx, const std::string& col_dir, const std::vector<std::vector<uint64_t> > patterns)
{
    using clock = std::chrono::high_resolution_clock;
    double perpelxity = 0;
    double sentenceprob = 0;
    int M = 0;
    std::chrono::nanoseconds total_time(0);
    for (const auto& pattern : patterns) {
        M += pattern.size() - 1; // -1 for discarding <s>
        // run the query
        auto start = clock::now();
        double sentenceprob = run_query_knm(idx, pattern);
        auto stop = clock::now();
        perplexity + = log10(sentenceprob);
        // output score
        std::copy(pattern.begin(), pattern.end(), std::ostream_iterator<uint64_t>(std::cout, " "));
        std::cout << " -> " << sentenceprob;
        total_time += (stop - start);
    }
    std::cout << "time in milliseconds = "
              << std::chrono::duration_cast<std::chrono::microseconds>(total_time).count() / 1000.0f
              << " ms" << endl;
    perplexity = (1 / (double)M) * perplexity;
    perplexity = pow(10, (-perplexity));
    std::cout << "Perplexity = " << perplexity << endl;
}

int main(int argc, const char* argv[])
{
    using clock = std::chrono::high_resolution_clock;

    /* parse command line */
    cmdargs_t args = parse_args(argc, argv);

    /* create collection dir */
    utils::create_directory(args.collection_dir);

    /* load index */
    using csa_type = sdsl::csa_sada_int<>;
    using cst_type = sdsl::cst_sct3<csa_type>;
    index_succinct<cst_type> idx;
    auto index_file = args.collection_dir + "/index/index-" + sdsl::util::class_to_hash(idx) + ".sdsl";
    if (utils::file_exists(index_file)) {
        std::cout << "loading index from file '" << index_file << "'" << std::endl;
        sdsl::load_from_file(idx, index_file);
    } else {
        std::cerr << "index does not exist. build it first" << std::endl;
        return EXIT_FAILURE;
    }

    /* parse pattern file */
    std::vector<std::vector<uint64_t> > patterns;
    if (utils::file_exists(args.pattern_file)) {
        std::ifstream ifile(args.pattern_file);
        std::cout << "reading input file '" << args.pattern_file << "'" << std::endl;
        std::string line;
        while (std::getline(ifile, line)) {
            std::vector<uint64_t> tokens;
            std::istringstream iss(line);
            std::string word;
            while (std::getline(iss, word, ' ')) {
                uint64_t num = std::stoull(word);
                tokens.push_back(num);
            }
            patterns.push_back(tokens); //each pattern is a sentence <s> w1 w2 w3 ... </s>
        }
    } else {
        std::cerr << "cannot read pattern file '" << args.pattern_file << "'" << std::endl;
        return EXIT_FAILURE;
    }

    {
        run_queries(idx, args.collection_dir, patterns);
    }
    return 0;
}
