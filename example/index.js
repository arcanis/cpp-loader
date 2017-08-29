import { getLevenshtein, getAllPermutations } from './lib.cc';

let text = `Hello`;

for (let permutation of getAllPermutations(text))
    console.log(`levenshtein(${permutation}) = ${getLevenshtein(text, permutation)}`);
