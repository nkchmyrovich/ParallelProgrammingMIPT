template<class T> class blockmatrix {
public:
  blockmatrix(size_t lines, size_t columns): lines(lines), columns(columns) { // subject to change
    bigbody = new T[lines * columns];
    body = new T*[lines];
    for (size_t i = 0; i < lines; i++) {
      body[i] = bigbody + i*columns;
    }
  }
   
  ~blockmatrix() { // subject to change
    delete [] body;
    delete [] bigbody;
  }
   
  T * operator()(size_t line, size_t col) { // *acc(0,0) = 2.34; *acc(i,j) = *acc(i,j+1);
    return body[line]+col;
  }
   
  T * operator()(size_t line, size_t col) const { // *acc(0,0) = 2.34; *acc(i,j) = *acc(i,j+1);
    return body[line]+col;
  }
   
  bool mul(blockmatrix const &left, blockmatrix const &right) {
    if (left.columns != right.lines) return false;
    if (lines != left.lines) return false;
    if (columns != right.columns) return false;
    for (size_t i = 0; i < lines; i++) {
      for (size_t j = 0; j < columns; j++) {
        T acc = (T)0;
        for (size_t k = 0; k < left.columns; k++) {
          acc += *left(i,k) * *right(k,j);
        }
        *(*this)(i,j) = acc;
      }
    }
    return true;
  }
  
  bool add(blockmatrix const &left, blockmatrix const &right);
  static void tune(size_t arg) { // subject for change
    ;
  }
private:
  T ** body;
  T *bigbody;
  size_t lines, columns;
};

