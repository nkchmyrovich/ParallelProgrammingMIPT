#include <vector>
#define OP_SIZE 25

template <class T>
class Block {
public:
    Block mul(const Block& l_block) {
        Block res_matr;
        for (int i = 0; i < block_size; i++) {
            for (int j = 0; j < block_size; j++) {
                res_matr.body[i][j] = 0;
                for (int k = 0; k < block_size; k++) {
                    res_matr.body[i][j] += body[i][k] * l_block.body[k][j];
                }
            }
        }
        return res_matr;
    }

    Block& operator+=(const Block& that_block) {
        for (int i = 0; i < block_size; i++) {
            for (int j = 0; j < block_size; j++) {
                body[i][j] += that_block.body[i][j];
            }
        }
        return *this;
    }

    Block() : block_size(OP_SIZE) {
        body = new T* [block_size];
        for (int i = 0; i < block_size; i++) {
            body[i] = new T [block_size];
        }
    }

    ~Block() {
    
    }
    T** body;
private:
    int block_size;
};


template <class T>
class BlockMatrix {
public:
    bool mul(const BlockMatrix& left, const BlockMatrix& right);
    T& operator()(int row, int col);
    BlockMatrix(int rows, int cols) : block_size(OP_SIZE), matrix_n_rows(rows), matrix_n_cols(cols), blocks() {
        block_n_rows = matrix_n_rows / block_size + (matrix_n_rows % block_size > 0);
        block_n_cols = matrix_n_cols / block_size + (matrix_n_cols % block_size > 0);
        blocks = new Block<T> [block_n_rows * block_n_cols];
    }
    ~BlockMatrix() {
        
    }
    Block<T>* blocks;
    int matrix_n_rows;
    int matrix_n_cols;
    int block_n_rows;
    int block_n_cols;
    int block_size;
};


template <class T>
T& BlockMatrix<T>::operator() (int row, int col) {
    int block_row = row / block_size;
    int block_col = col / block_size;
    int row_in_block = row % block_size;
    int col_in_block = col % block_size;
    return blocks[block_row * block_n_cols + block_col].body[row_in_block][col_in_block];
}


template <class T>
bool BlockMatrix<T>::mul(const BlockMatrix& left, const BlockMatrix& right) {
    if (left.matrix_n_cols != right.matrix_n_rows) return false;
    if (matrix_n_rows != left.matrix_n_rows) return false;
    if (matrix_n_cols != right.matrix_n_cols) return false;
    for (int i = 0; i < block_n_rows; i++) {
        for (int j = 0; j < block_n_cols; j++) {
            Block<T> sum;
            for (int k = 0; k < left.block_n_cols; k++) {
                sum += left.blocks[i * left.block_n_cols + k].mul(right.blocks[k * right.block_n_cols + j]);
            }
            blocks[i * block_n_cols + j] = sum;
        }
    }
    return true;
}
