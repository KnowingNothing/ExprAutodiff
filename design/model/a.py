import tvm as core
import numpy as np


def auto_schedule(sch):
    return sch

###############################
# a graph is a huge program with 
# a list of inputs and a list of
# outputs
class Graph:
    def __call__(self, *args):
        pass


def autograd(graph):
    return graph


class Placeholder:
    def __init__(self, shape, dtype, name):
        self.shape = shape
        self.dtype = dtype
        self.name = name
        self.placeholder = core.placeholder(shape, dtype=dtype, name=name)

    def __getitem__(self, *args):
        """
        overload slicing
        """
        pass

class Dataholder:
    def __init__(self, data, device_type="llvm", device_id=0):
        self.device_type = device_type
        self.device_id = device_id
        self.dataholder = core.nd.array(data)

    def __getitem__(self, *args):
        """
        overload slicing
        """
        pass

    def to(self, device_type, device_id):
        """
        change the place of data
        """
        pass

    def set_producer(self, func):
        """
        record the producer, a concrete function
        """
        pass

    def get_graph(self):
        """
        get the graph
        """
        #  |------------------------------------------------------------------|
        #  |                                                                  |
        #  v                                                                  |
        # self -> producer -> abstract_outputs -> operation -> input_tensors -|
        return Graph()

    def backward(self):
        pass

    def step(self):
        pass


class AbstractFunction:
    def make(self):
        raise NotImplementedError()


class ConcreteFunction:
    def __init__(self, func, abstract_func, abstract_inputs, abstract_outputs, concrete_outputs):
        self.func = func
        self.abstract_func = abstract_func
        self.abstract_inputs = abstact_inputs
        self.abstract_outputs = abstract_outputs
        self.concrete_outputs = concrete_outputs

    def __call__(self, *args):
        # need to record concrete tensors
        self.__record_inputs(*args)
        self.func(*args, *self.concrete_outputs)
        if len(self.concrete_outputs) == 1:
            return self.concrete_outputs[0]
        else:
            return self.concrete_outputs

    def __record_inputs(self, *args):
        pass

#################################################
# we want a abstract function to be also callable
# because the user may find it tedious to write
# a lot of `make`. Depend on the parameter type
# the behaviour if `__call__` is different.
# If the parameters are abstract tensors, i.e., 
# placeholders, the return placeholders; if the
# paramters are concrete tensors, i.e., dataholders,
# then return dataholders. 
# 
# In our design, a tensor is responsible to record
# the graph structure. 
# - An abstract tensor records the operation that 
#   creates it, the operation has a trace of its 
#   input_tensors (also abstract), so a graph
#   structure is maintained.
# - A concrete tensor records the concrete function
#   that results it. A concrete function records
#   its abstract function and abstract inputs/outputs.
#   Tracing from the abstract outputs, we can
#   retrieve graph structure.
#

class GEMMFunction(AbstractFunction):
    def __ini__(self):
        super(GEMMFunction, self).__init__()

    def _gemm(A, B):
        k = core.reduce_axis((0, A.shape[1]), name="k")

        C = core.compute((A.shape[0], B.shape[1]), lambda i, j: core.sum(A[i, k] * B[k, j], axis=[k]), name="C")

        return C


    def make(self, A, B, target="llvm"):
        """
        A = core.placeholder((M, K), dtype=dtype, name="A")
        B = core.placeholder((K, N), dtype=dtype, name="B")
        """

        C = self._gemm(A, B)

        s = core.create_schedule([C.op])
        
        ##################################
        # do some optimization on operator level
        s = auto_schedule(s)

        func = core.build(s, [A, B, C], target)

        tC = Dataholder(np.zeros([int(A.shape[0]), int(B.shape[1])]))

        # record which function produces it
        tC.set_producer(func)

        return ConcreteFunction(func, self, [A, B], [C], [tC])

    def __call__(self, A, B, target="llvm", device_id=0):
        if isinstance(A, Placeholder):
            assert isinstance(B, Placeholder)
            return self._gemm(A, B)
        elif isinstance(A. Dataholder):
            assert isinstance(B, Dataholder)
            pA = Placeholder(A.shape, dtype=dtype, name="A")
            pB = Placeholder(B.shape, dtype=dtype, name="B")
            func = self.make(pA, pB, target)
            if A.device_type != target or A.device_id != device_id:
                A = A.to(target, device_id)
            if B.device_type != target or B.device_id != device_id:
                B = B.to(target, device_id)
            return func(A, B)


#################################################
# you can make a concrete GEMM from abstract GEMM

abs_gemm = GEMMFunction()
A = Placeholder((M, K), dtype=dtype, name="A")
B = Placeholder((K, N), dtype=dtype, name="B")
cnt_gemm = abs_gemm.make(A, B)

#################################################
# you can call concrete GEMM with concrete data

tA = Dataholder(np.ramdom.uniform([1024, 1024]))
tB = Dataholder(np.random.uniform([1024, 1024]))

tC = cnt_gemm(tA, tB)

#################################################
# you can call abstract GEMM with concrete data
tC = abs_gemm(tA, tB)

#################################################
# you can dynamically make a graph by multiple calling
for i in range(10):
    tC = abs_gemm(tA, tB)
    tA = tB
    tB = tC

#################################################
# you can get the graph structure
f_graph = tC.get_graph()

#################################################
# you can do autograd to get a graph with
# backpropagation definition
fb_graph = autograd(f_graph)

#################################################
# you can do backpropagation by calling the graph
shape = [1024, 1024]
dout = Dataholder(np.random.uniform(shape))
fb_graph(dout)

#################################################
# another way to do the backpropagation
tC.backward()
tC.step()