#pragma once

#include "stdafx.h"

class HashHelper {
public:

	static std::string RandomKey(int size) {
		const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

		std::random_device randDev;
		std::mt19937 generator(randDev());

		std::uniform_int_distribution<int> distr(0, CHARACTERS.size() - 1);

		std::string result;
		for (int i = 0; i < size; i++) {
			result += CHARACTERS[distr(generator)];
		}

		return result;
	}

	static int RandomNumber(int min, int max) {
		std::random_device randDev;
		std::mt19937 generator(randDev());
		std::uniform_int_distribution<int> distr(min, max);
		return distr(generator);
	}

	static std::vector<uint8_t> StringToBytes(std::string input) {
		std::vector<uint8_t> res = std::vector<uint8_t>(input.begin(), input.end());
		res.push_back((uint8_t)'\0'); // Unreal Engine engine needs it to be null terminated or it will mess up the end of the string when decoding.
		return res;
	}

	static std::string BytesToString(std::vector<uint8_t> input) {
		return std::string(input.begin(), input.end());
	}
};