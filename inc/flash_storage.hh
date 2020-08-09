extern "C" {
#include "drivers/flash_S25FL127.h"
#include "drivers/flashram_spidma.h"
}

template<int sector, class Data>
struct FlashStorage {
	using data_t = Data;
	static constexpr int size_ = sFLASH_SPI_4K_SECTOR_SIZE;
	static constexpr int data_size_ = sizeof(data_t);
	// data size aligned to the next page boundary
	static constexpr int aligned_data_size_ =
		((data_size_ >> sFLASH_PAGESIZE_BITS) + 1) << sFLASH_PAGESIZE_BITS;
	static_assert(aligned_data_size_ < size_);
	static constexpr int cell_nr_ = size_ / aligned_data_size_;

	bool Read(data_t *data, int cell)
	{
		if (cell >= cell_nr_) return false;
		uint32_t addr = sFLASH_get_sector_addr(sector) + cell * aligned_data_size_;
		sFLASH_read_buffer(reinterpret_cast<uint8_t *>(data), addr, data_size_);
		return true;
	}

	bool Write(data_t *data, int cell)
	{
		if (cell >= cell_nr_) return false;
		uint32_t addr = sFLASH_get_sector_addr(sector) + cell * aligned_data_size_;
		sFLASH_write_buffer(reinterpret_cast<uint8_t *>(data), addr, data_size_);
		return true;
	}

	void Erase()
	{
		sFLASH_erase_sector(sFLASH_get_sector_addr(sector));
	}
	// Verify all bits are 1's
	bool IsWriteable(int cell)
	{
		if (cell >= cell_nr_) return false;
		uint8_t check[data_size_];
		if (Read(reinterpret_cast<data_t *>(check), cell)) {
			for (int i = 0; i < data_size_; i++) {
				if (check[i] != 0xFF) return false;
			}
			return true;
		}
		return false;
	}

	// simple wrappers to read/write in 1st cell
	bool Read(data_t *data)
	{
		return Read(data, 0);
	}
	bool Write(data_t *data)
	{
		return Erase() && Write(data, 0);
	}
};
