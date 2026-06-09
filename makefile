CXX = g++

CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

# Flag untuk menghubungkan library eksternal
LDFLAGS = -lsqlite3

# Mencari semua file .cpp di dalam folder src/
SRCS = $(wildcard src/*.cpp)

# Mengubah ekstensi .cpp menjadi .o untuk proses kompilasi sementara
OBJS = $(SRCS:.cpp=.o)

# Nama file eksekusi akhir
TARGET = admin

# Aturan utama: ketika perintah 'make' dijalankan, ia akan membuat target
all: $(TARGET)

# Proses penautan (Linking) file .o menjadi aplikasi akhir
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Aturan untuk membersihkan file hasil kompilasi agar tidak masuk ke Git
clean:
	rm -f $(OBJS) $(TARGET)