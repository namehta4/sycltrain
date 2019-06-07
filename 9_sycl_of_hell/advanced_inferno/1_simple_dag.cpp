#include <iostream>
#include <vector>
#include <CL/sycl.hpp>

template <typename dataW, typename dataR>
class memcopy_kernel{
 public:
   memcopy_kernel(dataW accessorW_, dataR accessorR_): accessorW{accessorW_},  accessorR{accessorR_}  {}
   void operator()(cl::sycl::item<1> idx) { accessorW[idx.get_id()] = accessorR[idx.get_id()] + 1; }
 private:
   dataW accessorW;
   dataR accessorR;
};

void f_copy(cl::sycl::handler& cgh, int global_range, cl::sycl::buffer<cl::sycl::cl_int, 1> bufferW, 
                                                      cl::sycl::buffer<cl::sycl::cl_int, 1> bufferR) {
    auto accessorW = bufferW.get_access<cl::sycl::access::mode::discard_write>(cgh);
    auto accessorR = bufferR.get_access<cl::sycl::access::mode::read>(cgh);
    cgh.parallel_for(cl::sycl::range<1>(global_range), memcopy_kernel(accessorW, accessorR));
}

int main(int argc, char** argv) {

   const auto global_range =  (size_t) atoi(argv[1]);

   // Crrate array
   std::vector<int> A(global_range, 0);

  // Selectors determine which device kernels will be dispatched to.
  cl::sycl::default_selector selector;
  // Create your own or use `{cpu,gpu,accelerator}_selector`
  {
  cl::sycl::queue myQueue(selector);
  std::cout << "Running on "
            << myQueue.get_device().get_info<cl::sycl::info::device::name>()
            << "\n";
  
  //   _      __ 
  //  | \ /\ /__ 
  //  |_//--\\_| 

  cl::sycl::buffer<cl::sycl::cl_int, 1> bufferA(A.data(), global_range);
  cl::sycl::buffer<cl::sycl::cl_int, 1> bufferB(global_range);
  cl::sycl::buffer<cl::sycl::cl_int, 1> bufferC(global_range);

  myQueue.submit(std::bind(f_copy, std::placeholders::_1, global_range, bufferB, bufferA));
  myQueue.submit(std::bind(f_copy, std::placeholders::_1, global_range, bufferC, bufferB));
  myQueue.submit(std::bind(f_copy, std::placeholders::_1, global_range, bufferA, bufferC));

  }  // End of scope, wait for the queued work to stop.
 
 for (size_t i = 0; i < global_range; i++)
        std::cout<< "A[ " << i <<" ] = " << A[i] << std::endl;
  return 0;
}
