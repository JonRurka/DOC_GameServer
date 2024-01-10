#pragma once

#include "../stdafx.h"

typedef unsigned int Remove;

#define Remove_CMD			1
#define Remove_LENGTH		2
#define Remove_IS_SERVER	1
#define Remove_UDP_ID		2

class BufferUtils {
public:
	static std::vector<uint8_t> AddLength(std::vector<uint8_t>& data) {
		uint16_t len = data.size();
		uint8_t* len_buff = (uint8_t*)&len;
		uint8_t lo = len_buff[0];// len & 0xFF;
		uint8_t hi = len_buff[1];// len >> 8;
		std::vector<uint8_t> len_v;// {lo, hi};
		len_v.push_back(lo);
		len_v.push_back(hi);

		return Add(len_v, data);
	}

	static std::vector<uint8_t> Add_UDP_ID(uint16_t udp_id, std::vector<uint8_t> data) {
		//uint16_t len = data.size();
		uint8_t* udp_id_buff = (uint8_t*)&udp_id;
		uint8_t lo = udp_id_buff[0];// len & 0xFF;
		uint8_t hi = udp_id_buff[1];// len >> 8;
		std::vector<uint8_t> id_v;// {lo, hi};
		id_v.push_back(lo);
		id_v.push_back(hi);

		return Add(id_v, data);
	}

	static std::vector<uint8_t> RemoveFront(Remove numToRemove, std::vector<uint8_t> origin)
	{
		std::vector<uint8_t> res;
		if ((int)numToRemove > origin.size())
		{
			return res;
			//throw new Exception(string.Format("RemoveFront: received remove length ({0}) longer than buffer: {1}", (int)numToRemove, BitConverter.ToString(origin)));
		}

		res.assign(origin.begin() + numToRemove, origin.end());
		return res;
	}

	static std::vector<uint8_t> AddFirst(uint8_t byteToAdd, std::vector<uint8_t> origin)
	{
		origin.insert(origin.begin(), byteToAdd);
		return origin;
	}

	static std::vector<uint8_t> Add(std::vector<uint8_t> buffer_1, std::vector<uint8_t> buffer_2) {
		
		std::vector<std::vector<uint8_t>> buffers;
		buffers.push_back(buffer_1);
		buffers.push_back(buffer_2);
		return Add(buffers);
	}

	static std::vector<uint8_t> Add(std::vector<std::vector<uint8_t>>& buffers) {
		std::vector<uint8_t> res;
		for (int i = 0; i < buffers.size(); i++)
		{
			res.insert(res.end(), buffers[i].begin(), buffers[i].end());
		}
		return res;
	}

	static std::vector<uint8_t> Add(std::vector<uint8_t>* buffers_arr, int num) {

		std::vector<std::vector<uint8_t>> buffers;
		for (int i = 0; i < num; i++) {
			buffers.push_back(buffers_arr[i]);
		}
		return Add(buffers);
	}
};