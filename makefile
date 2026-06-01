# Menentukan compiler yang digunakan
CXX = g++

# Flag kompilasi: menggunakan C++17 dan mengaktifkan peringatan kode
CXXFLAGS = -std=c++17 -Wall -Wextra

# Flag untuk menghubungkan library eksternal (contoh: SQLite)
LDFLAGS = -lsqlite3

# Mencari semua file .cpp di dalam folder src/
SRCS = $(wildcard src/*.cpp)

# Mengubah ekstensi .cpp menjadi .o untuk proses kompilasi sementara
OBJS = $(SRCS:.cpp=.o)

# Nama file eksekusi akhir (sesuai spesifikasi CLI)
TARGET = admin

# Aturan utama: ketika perintah 'make' dijalankan, ia akan membuat target
all: $(TARGET)

# Proses penautan (Linking) file .o menjadi aplikasi akhir
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Aturan untuk membersihkan file hasil kompilasi agar tidak masuk ke Git
clean:
	rm -f $(OBJS) $(TARGET)