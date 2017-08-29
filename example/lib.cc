#include <algorithm>
#include <string>
#include <vector>

std::vector<std::string> getAllPermutations(std::string string)
{
    std::vector<std::string> permutations;

    std::sort(string.begin(), string.end());

    do {
        permutations.emplace_back(string);
    } while (std::next_permutation(string.begin(), string.end()));

    return permutations;
}

unsigned int getLevenshtein(std::string const & s1, std::string const & s2)
{
    auto len1 = s1.size();
    auto len2 = s2.size();

    std::vector<std::vector<unsigned int>> d(len1 + 1, std::vector<unsigned int>(len2 + 1));

    d[0][0] = 0;

    for(decltype(len1) i = 1; i <= len1; ++i)
        d[i][0] = i;

    for(decltype(len2) i = 1; i <= len2; ++i)
        d[0][i] = i;

    for(decltype(len1) i = 1; i <= len1; ++i)
        for(decltype(len2) j = 1; j <= len2; ++j)
            d[i][j] = std::min({ d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1) });

    return d[len1][len2];
}
