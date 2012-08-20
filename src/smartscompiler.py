# This file was automatically generated by SWIG (http://www.swig.org).
# Version 2.0.4
#
# Do not make changes to this file unless you know what you are doing--modify
# the SWIG interface file instead.



from sys import version_info
if version_info >= (3,0,0):
    new_instancemethod = lambda func, inst, cls: _smartscompiler.SWIG_PyInstanceMethod_New(func)
else:
    from new import instancemethod as new_instancemethod
if version_info >= (2,6,0):
    def swig_import_helper():
        from os.path import dirname
        import imp
        fp = None
        try:
            fp, pathname, description = imp.find_module('_smartscompiler', [dirname(__file__)])
        except ImportError:
            import _smartscompiler
            return _smartscompiler
        if fp is not None:
            try:
                _mod = imp.load_module('_smartscompiler', fp, pathname, description)
            finally:
                fp.close()
            return _mod
    _smartscompiler = swig_import_helper()
    del swig_import_helper
else:
    import _smartscompiler
del version_info
try:
    _swig_property = property
except NameError:
    pass # Python < 2.2 doesn't have 'property'.
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'SwigPyObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError(name)

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

try:
    _object = object
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0


def _swig_setattr_nondynamic_method(set):
    def set_attr(self,name,value):
        if (name == "thisown"): return self.this.own(value)
        if hasattr(self,name) or (name == "this"):
            set(self,name,value)
        else:
            raise AttributeError("You cannot add attributes to %s" % self)
    return set_attr


try:
    import weakref
    weakref_proxy = weakref.proxy
except:
    weakref_proxy = lambda x: x


class SwigPyIterator(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError("No constructor defined - class is abstract")
    __repr__ = _swig_repr
    __swig_destroy__ = _smartscompiler.delete_SwigPyIterator
    def __iter__(self): return self
SwigPyIterator.value = new_instancemethod(_smartscompiler.SwigPyIterator_value,None,SwigPyIterator)
SwigPyIterator.incr = new_instancemethod(_smartscompiler.SwigPyIterator_incr,None,SwigPyIterator)
SwigPyIterator.decr = new_instancemethod(_smartscompiler.SwigPyIterator_decr,None,SwigPyIterator)
SwigPyIterator.distance = new_instancemethod(_smartscompiler.SwigPyIterator_distance,None,SwigPyIterator)
SwigPyIterator.equal = new_instancemethod(_smartscompiler.SwigPyIterator_equal,None,SwigPyIterator)
SwigPyIterator.copy = new_instancemethod(_smartscompiler.SwigPyIterator_copy,None,SwigPyIterator)
SwigPyIterator.next = new_instancemethod(_smartscompiler.SwigPyIterator_next,None,SwigPyIterator)
SwigPyIterator.__next__ = new_instancemethod(_smartscompiler.SwigPyIterator___next__,None,SwigPyIterator)
SwigPyIterator.previous = new_instancemethod(_smartscompiler.SwigPyIterator_previous,None,SwigPyIterator)
SwigPyIterator.advance = new_instancemethod(_smartscompiler.SwigPyIterator_advance,None,SwigPyIterator)
SwigPyIterator.__eq__ = new_instancemethod(_smartscompiler.SwigPyIterator___eq__,None,SwigPyIterator)
SwigPyIterator.__ne__ = new_instancemethod(_smartscompiler.SwigPyIterator___ne__,None,SwigPyIterator)
SwigPyIterator.__iadd__ = new_instancemethod(_smartscompiler.SwigPyIterator___iadd__,None,SwigPyIterator)
SwigPyIterator.__isub__ = new_instancemethod(_smartscompiler.SwigPyIterator___isub__,None,SwigPyIterator)
SwigPyIterator.__add__ = new_instancemethod(_smartscompiler.SwigPyIterator___add__,None,SwigPyIterator)
SwigPyIterator.__sub__ = new_instancemethod(_smartscompiler.SwigPyIterator___sub__,None,SwigPyIterator)
SwigPyIterator_swigregister = _smartscompiler.SwigPyIterator_swigregister
SwigPyIterator_swigregister(SwigPyIterator)

class vector_bond(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __iter__(self): return self.iterator()
    def __init__(self, *args): 
        _smartscompiler.vector_bond_swiginit(self,_smartscompiler.new_vector_bond(*args))
    __swig_destroy__ = _smartscompiler.delete_vector_bond
vector_bond.iterator = new_instancemethod(_smartscompiler.vector_bond_iterator,None,vector_bond)
vector_bond.__nonzero__ = new_instancemethod(_smartscompiler.vector_bond___nonzero__,None,vector_bond)
vector_bond.__bool__ = new_instancemethod(_smartscompiler.vector_bond___bool__,None,vector_bond)
vector_bond.__len__ = new_instancemethod(_smartscompiler.vector_bond___len__,None,vector_bond)
vector_bond.pop = new_instancemethod(_smartscompiler.vector_bond_pop,None,vector_bond)
vector_bond.__getslice__ = new_instancemethod(_smartscompiler.vector_bond___getslice__,None,vector_bond)
vector_bond.__setslice__ = new_instancemethod(_smartscompiler.vector_bond___setslice__,None,vector_bond)
vector_bond.__delslice__ = new_instancemethod(_smartscompiler.vector_bond___delslice__,None,vector_bond)
vector_bond.__delitem__ = new_instancemethod(_smartscompiler.vector_bond___delitem__,None,vector_bond)
vector_bond.__getitem__ = new_instancemethod(_smartscompiler.vector_bond___getitem__,None,vector_bond)
vector_bond.__setitem__ = new_instancemethod(_smartscompiler.vector_bond___setitem__,None,vector_bond)
vector_bond.append = new_instancemethod(_smartscompiler.vector_bond_append,None,vector_bond)
vector_bond.empty = new_instancemethod(_smartscompiler.vector_bond_empty,None,vector_bond)
vector_bond.size = new_instancemethod(_smartscompiler.vector_bond_size,None,vector_bond)
vector_bond.clear = new_instancemethod(_smartscompiler.vector_bond_clear,None,vector_bond)
vector_bond.swap = new_instancemethod(_smartscompiler.vector_bond_swap,None,vector_bond)
vector_bond.get_allocator = new_instancemethod(_smartscompiler.vector_bond_get_allocator,None,vector_bond)
vector_bond.begin = new_instancemethod(_smartscompiler.vector_bond_begin,None,vector_bond)
vector_bond.end = new_instancemethod(_smartscompiler.vector_bond_end,None,vector_bond)
vector_bond.rbegin = new_instancemethod(_smartscompiler.vector_bond_rbegin,None,vector_bond)
vector_bond.rend = new_instancemethod(_smartscompiler.vector_bond_rend,None,vector_bond)
vector_bond.pop_back = new_instancemethod(_smartscompiler.vector_bond_pop_back,None,vector_bond)
vector_bond.erase = new_instancemethod(_smartscompiler.vector_bond_erase,None,vector_bond)
vector_bond.push_back = new_instancemethod(_smartscompiler.vector_bond_push_back,None,vector_bond)
vector_bond.front = new_instancemethod(_smartscompiler.vector_bond_front,None,vector_bond)
vector_bond.back = new_instancemethod(_smartscompiler.vector_bond_back,None,vector_bond)
vector_bond.assign = new_instancemethod(_smartscompiler.vector_bond_assign,None,vector_bond)
vector_bond.resize = new_instancemethod(_smartscompiler.vector_bond_resize,None,vector_bond)
vector_bond.insert = new_instancemethod(_smartscompiler.vector_bond_insert,None,vector_bond)
vector_bond.reserve = new_instancemethod(_smartscompiler.vector_bond_reserve,None,vector_bond)
vector_bond.capacity = new_instancemethod(_smartscompiler.vector_bond_capacity,None,vector_bond)
vector_bond_swigregister = _smartscompiler.vector_bond_swigregister
vector_bond_swigregister(vector_bond)

SE_NullType = _smartscompiler.SE_NullType
AE_NullType = _smartscompiler.AE_NullType
BE_NullType = _smartscompiler.BE_NullType
SE_AndHigh = _smartscompiler.SE_AndHigh
AE_AndHigh = _smartscompiler.AE_AndHigh
BE_AndHigh = _smartscompiler.BE_AndHigh
SE_AndLow = _smartscompiler.SE_AndLow
AE_AndLow = _smartscompiler.AE_AndLow
BE_AndLow = _smartscompiler.BE_AndLow
SE_Or = _smartscompiler.SE_Or
AE_Or = _smartscompiler.AE_Or
BE_Or = _smartscompiler.BE_Or
SE_Not = _smartscompiler.SE_Not
AE_Not = _smartscompiler.AE_Not
BE_Not = _smartscompiler.BE_Not
AE_Recursive = _smartscompiler.AE_Recursive
SE_True = _smartscompiler.SE_True
AE_True = _smartscompiler.AE_True
BE_True = _smartscompiler.BE_True
SE_False = _smartscompiler.SE_False
AE_False = _smartscompiler.AE_False
BE_False = _smartscompiler.BE_False
AE_Aromatic = _smartscompiler.AE_Aromatic
AE_Aliphatic = _smartscompiler.AE_Aliphatic
AE_Cyclic = _smartscompiler.AE_Cyclic
AE_Acyclic = _smartscompiler.AE_Acyclic
AE_Isotope = _smartscompiler.AE_Isotope
AE_Element = _smartscompiler.AE_Element
AE_AromaticElement = _smartscompiler.AE_AromaticElement
AE_AliphaticElement = _smartscompiler.AE_AliphaticElement
AE_HydrogenCount = _smartscompiler.AE_HydrogenCount
AE_ImplicitHydrogenCount = _smartscompiler.AE_ImplicitHydrogenCount
AE_Charge = _smartscompiler.AE_Charge
AE_Connectivity = _smartscompiler.AE_Connectivity
AE_Degree = _smartscompiler.AE_Degree
AE_Valence = _smartscompiler.AE_Valence
AE_NumRings = _smartscompiler.AE_NumRings
AE_RingConnectivity = _smartscompiler.AE_RingConnectivity
AE_RingSize = _smartscompiler.AE_RingSize
AE_Hybridization = _smartscompiler.AE_Hybridization
AE_Chiral = _smartscompiler.AE_Chiral
BE_Default = _smartscompiler.BE_Default
BE_Single = _smartscompiler.BE_Single
BE_Double = _smartscompiler.BE_Double
BE_Triple = _smartscompiler.BE_Triple
BE_Quadruple = _smartscompiler.BE_Quadruple
BE_Aromatic = _smartscompiler.BE_Aromatic
BE_Ring = _smartscompiler.BE_Ring
BE_Up = _smartscompiler.BE_Up
BE_Down = _smartscompiler.BE_Down
BE_UpUnspecified = _smartscompiler.BE_UpUnspecified
BE_DownUnspecified = _smartscompiler.BE_DownUnspecified
class SmartsAtomExpr(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    type = _swig_property(_smartscompiler.SmartsAtomExpr_type_get, _smartscompiler.SmartsAtomExpr_type_set)
    def __init__(self): 
        _smartscompiler.SmartsAtomExpr_swiginit(self,_smartscompiler.new_SmartsAtomExpr())
    __swig_destroy__ = _smartscompiler.delete_SmartsAtomExpr
SmartsAtomExpr_swigregister = _smartscompiler.SmartsAtomExpr_swigregister
SmartsAtomExpr_swigregister(SmartsAtomExpr)

class SmartsBondExpr(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    type = _swig_property(_smartscompiler.SmartsBondExpr_type_get, _smartscompiler.SmartsBondExpr_type_set)
    def __init__(self): 
        _smartscompiler.SmartsBondExpr_swiginit(self,_smartscompiler.new_SmartsBondExpr())
    __swig_destroy__ = _smartscompiler.delete_SmartsBondExpr
SmartsBondExpr_swigregister = _smartscompiler.SmartsBondExpr_swigregister
SmartsBondExpr_swigregister(SmartsBondExpr)

class SmartsBond(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        _smartscompiler.SmartsBond_swiginit(self,_smartscompiler.new_SmartsBond(*args))
    src = _swig_property(_smartscompiler.SmartsBond_src_get, _smartscompiler.SmartsBond_src_set)
    dst = _swig_property(_smartscompiler.SmartsBond_dst_get, _smartscompiler.SmartsBond_dst_set)
    grow = _swig_property(_smartscompiler.SmartsBond_grow_get, _smartscompiler.SmartsBond_grow_set)
    __swig_destroy__ = _smartscompiler.delete_SmartsBond
SmartsBond_swigregister = _smartscompiler.SmartsBond_swigregister
SmartsBond_swigregister(SmartsBond)

class SmartsPattern(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    numAtoms = _swig_property(_smartscompiler.SmartsPattern_numAtoms_get, _smartscompiler.SmartsPattern_numAtoms_set)
    bonds = _swig_property(_smartscompiler.SmartsPattern_bonds_get, _smartscompiler.SmartsPattern_bonds_set)
    ischiral = _swig_property(_smartscompiler.SmartsPattern_ischiral_get, _smartscompiler.SmartsPattern_ischiral_set)
    EvalAtomExpr = _swig_property(_smartscompiler.SmartsPattern_EvalAtomExpr_get, _smartscompiler.SmartsPattern_EvalAtomExpr_set)
    EvalBondExpr = _swig_property(_smartscompiler.SmartsPattern_EvalBondExpr_get, _smartscompiler.SmartsPattern_EvalBondExpr_set)
    def __init__(self): 
        _smartscompiler.SmartsPattern_swiginit(self,_smartscompiler.new_SmartsPattern())
    __swig_destroy__ = _smartscompiler.delete_SmartsPattern
SmartsPattern.CallEvalExpr = new_instancemethod(_smartscompiler.SmartsPattern_CallEvalExpr,None,SmartsPattern)
SmartsPattern_swigregister = _smartscompiler.SmartsPattern_swigregister
SmartsPattern_swigregister(SmartsPattern)

class PythonSmartsPattern(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    numAtoms = _swig_property(_smartscompiler.PythonSmartsPattern_numAtoms_get, _smartscompiler.PythonSmartsPattern_numAtoms_set)
    bonds = _swig_property(_smartscompiler.PythonSmartsPattern_bonds_get, _smartscompiler.PythonSmartsPattern_bonds_set)
    ischiral = _swig_property(_smartscompiler.PythonSmartsPattern_ischiral_get, _smartscompiler.PythonSmartsPattern_ischiral_set)
    EvalAtomExpr = _swig_property(_smartscompiler.PythonSmartsPattern_EvalAtomExpr_get, _smartscompiler.PythonSmartsPattern_EvalAtomExpr_set)
    EvalBondExpr = _swig_property(_smartscompiler.PythonSmartsPattern_EvalBondExpr_get, _smartscompiler.PythonSmartsPattern_EvalBondExpr_set)
    def __init__(self): 
        _smartscompiler.PythonSmartsPattern_swiginit(self,_smartscompiler.new_PythonSmartsPattern())
    __swig_destroy__ = _smartscompiler.delete_PythonSmartsPattern
PythonSmartsPattern.CallEvalExpr = new_instancemethod(_smartscompiler.PythonSmartsPattern_CallEvalExpr,None,PythonSmartsPattern)
PythonSmartsPattern_swigregister = _smartscompiler.PythonSmartsPattern_swigregister
PythonSmartsPattern_swigregister(PythonSmartsPattern)

class SmartsMatcher(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self): 
        _smartscompiler.SmartsMatcher_swiginit(self,_smartscompiler.new_SmartsMatcher())
    __swig_destroy__ = _smartscompiler.delete_SmartsMatcher
SmartsMatcher.Match = new_instancemethod(_smartscompiler.SmartsMatcher_Match,None,SmartsMatcher)
SmartsMatcher_swigregister = _smartscompiler.SmartsMatcher_swigregister
SmartsMatcher_swigregister(SmartsMatcher)

class OpenBabelSmartsMatcher(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self): 
        _smartscompiler.OpenBabelSmartsMatcher_swiginit(self,_smartscompiler.new_OpenBabelSmartsMatcher())
    __swig_destroy__ = _smartscompiler.delete_OpenBabelSmartsMatcher
OpenBabelSmartsMatcher.GetPattern = new_instancemethod(_smartscompiler.OpenBabelSmartsMatcher_GetPattern,None,OpenBabelSmartsMatcher)
OpenBabelSmartsMatcher_swigregister = _smartscompiler.OpenBabelSmartsMatcher_swigregister
OpenBabelSmartsMatcher_swigregister(OpenBabelSmartsMatcher)



