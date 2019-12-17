import math
import tatsu
import os
from os import path

STDLIB = "StdElement"
grammar = open(path.join(STDLIB, "grammar.ebnf")).read()
model = tatsu.compile(grammar)

intrinsics = {
    ('add', 'operator', '+'),
    ('sub', 'operator', '-'),
    ('mul', 'operator', '*'),
    ('div', 'operator', '/'),
    ('mod', 'operator', '%'),
    ('pow', 'operator', '**'),
    ('sin', math.sin),
    ('ln', math.log),
    ('vec', 'vector'),
    #('zip', 'zip')
}

eval_context = { }
fn_suffix = "_make_this_name_long_to_fool_the_optimizer"
for x in intrinsics:
    if callable(x[1]):
        eval_context[x[0] + fn_suffix] = x[1]

def const_eval(s): 
    try:
        return str(eval(s, eval_context))
    except NameError:
        return s

def array_index(output_name, args, index_fn):
    if len(args) != 1:
        raise Exception("Array index function takes exactly one argument")
    index = args[0]
    if not isinstance(index, str):
        raise Exception("Array index function cannot take a function")
    return index_fn(index)

def evaluate_vector_base(length, output_name, args, index_fn):
    if output_name != None and args and len(args) > 0:
        raise Exception("Can't call and access a vector at the same time")
    if output_name == 'count':
        return str(length)
    if output_name == 'index':
        return lambda n, a: array_index(n, a, index_fn)
    else:
        raise Exception("Output does not exist: " + output_name)

def vector_index(elements, index):
    try:
        # Constant index
        index = int(index)
        return elements[index]
    except ValueError:
        # Variable index
        if isinstance(elements[0], str):
            retval = ''
            for i in range(len(elements)):
                if i == len(elements)-1:
                    retval += elements[i] + ')'*(len(elements)-1)
                else:
                    op = i == 0 and '<=' or '=='
                    retval += 'int(' + index + ')' + op + str(i) + ' and ' + elements[0] + ' or ('
            return retval
            # Alternative implementation with a tuple:
            #return '(' + ','.join(elements) + ')[' + index + ']'
        # Otherwise, it's a function:
        return lambda name, args, index=index, elements=elements: \
            vector_index([e(name, args) for e in elements], index)

def evaluate_vector(elements, output_name, args):
    return evaluate_vector_base(len(elements), output_name, args, lambda i: vector_index(elements, i))

# def evaluate_tuple(var, length, output_name, args):
#     return evaluate_vector_base(length, output_name, args, lambda i: var + '[' + i + ']')

# def zip_intrinsic(args):
#     elements = []
#     length = args[1]('count', None)
#     try:
#         length = int(length)
#     except ValueError:
#         raise Exception("Length of zip argument must be constant")
#     # TODO: Check lengths are all equal
#     zipper = args[0]
#     vectors = [v('index', None) for v in args[1:]]
#     for x in range(length):
#         elements.append()
#     return lambda n, a, elements=elements: evaluate_vector_base(len(elements), output_name, args, \
#         lambda i: zipper(None, [v(None, (str(i), )) for v in vectors]))

def aggregate_intrinsic(args):
    length = None
    try:
        length = int(args[0]('count', None))
    except ValueError:
        raise Exception("Length of aggregate argument must be constant")
    result = args[2]
    index = args[0]('index', None)
    for x in range(length):
        result = args[1](None, (result, index(None, (str(x),))))
    return result

def evaluate_intrinsic(block, output_name, args):
    if output_name != None:
        raise Exception(block[0] + " has only one output")
    if not args:
        raise Exception(block[0] + " still has some inputs")
    if block[1] == 'vector':
        return lambda name, new_args, args=args: evaluate_vector(args, name, new_args)
    # if block[1] == 'zip':
    #     return zip_intrinsic(args)
    if block[1] == 'aggregate':
        return aggregate_intrinsic(args)
    for a in args:
        if callable(a):
            raise Exception(block[0] + " given function as input, but values required")
    if block[1] == 'operator':
        return const_eval('(' + block[2].join(args) + ')')
    if block[1] == 'format':
        return const_eval(block[2].format(*args))
    else:
        return const_eval('(' + block[0] + fn_suffix + '(' + ','.join(args) + '))')

def new_context():
    context = {'drivers':{}, 'reflection':{}, 'interfaces':{}}
    for x in intrinsics:
       context['drivers'][x[0]] = lambda name, args, x=x: evaluate_intrinsic(x, name, args)
    for file in os.listdir(STDLIB):
        fullPath = path.join(STDLIB, file)
        if path.isfile(fullPath) and path.normcase(file).endswith(".ele"):
            add_file(context, fullPath)
    return context

def add_file(context, path):
    ast = model.parse(open(path, 'rt', -1, 'utf-8').read())
    for b in ast:
        block_dict = {
            'name': b.name,
            'inputs': b.inputs or (),
            'outputs': b.outputs,
            'statements_ast': b.statements,
            'drivers': {},
            'intrinsic': False,
            'parent': context,
            'ast': b
        }
        if b.statements:
            if b.name in context['drivers']:
                raise Exception('Multiple definitions of ' + b.name)
            context['drivers'][b.name] = \
                lambda name, args, block_dict=block_dict: evaluate_structural(block_dict, name, args)
            context['reflection'][b.name] = block_dict
        else:
            if b.name in context['interfaces']:
                raise Exception('Multiple definitions of interface ' + b.name)
            context['interfaces'][b.name] = block_dict

def compile_driver(block, driver_name):
    drivers = block['drivers']
    if driver_name in drivers:
        return drivers[driver_name]
    if 'lambda_expr' in block:
        return compile_driver(block['parent'], driver_name)
    statements_ast = block.get('statements_ast')
    if not statements_ast:
        raise Exception(driver_name + " not found")
    statement = [x for x in statements_ast if driver_name == x.target]
    if len(statement) == 0:
        if block['parent']:
            return compile_driver(block['parent'], driver_name)
        raise Exception(driver_name + " not found")
    if len(statement) > 1:
        raise Exception(driver_name + " has multiple drivers")
    statement = statement[0]
    drivers[driver_name] = compile_expr(block, statement.value)
    return drivers[driver_name]

def evaluate_structural_after_call(block, original_args, output_name, new_args):
    if new_args and len(new_args) > 0:
        raise Exception(block['name'] + " must have an output specified before calling again")
    return evaluate_structural(block, output_name, original_args)

def setup_drivers(block, arguments):
    block['drivers'].clear()
    # TODO: Detect circular
    if len(arguments) != len(block['inputs']):
        raise Exception("Expected " + str(len(block['inputs'])) + " arguments, but got " + str(len(arguments)))
    for i in range(len(arguments)):
        block['drivers'][block['inputs'][i].name] = arguments[i]

def evaluate_structural(block, output_name, arguments):
    if output_name != None and len(block['inputs']) > 0 and not arguments:
        raise Exception(block['name'] + " needs to be called first")
    if not arguments:
        arguments = ()
    if not output_name:
        # TODO: Put these as class methods or something
        if not block['outputs'] and block.get('ast') and block['ast'].returns and block['ast'].returns.type:
            iface = context['interfaces'].get(block['ast'].returns.type)
            if not iface:
                raise Exception('Interface not found: ' + block['ast'].returns.type)
            block['outputs'] = iface['outputs']
        if not block['outputs']:
            if 'lambda_expr' in block:
                setup_drivers(block, arguments)
                return compile_expr(block, block['lambda_expr'])
            output_name = "return"
        else:
            return lambda name, new_args, block=block, arguments=arguments: \
                evaluate_structural_after_call(block, arguments, name, new_args)
    setup_drivers(block, arguments)
    return compile_driver(block, output_name)

def compile_expr(block, expr_ast):
    if isinstance(expr_ast, str):
        try:
            return str(float(expr_ast))
        except ValueError:
            return compile_driver(block, expr_ast)
    if expr_ast.inputs:
        lambda_block = {
            'name': 'Lambda in ' + block['name'],
            'inputs': expr_ast.inputs,
            'outputs': False,
            'lambda_expr': expr_ast.returns,
            'drivers': {},
            'intrinsic': False,
            'parent': block,
        }
        return lambda name, arguments, lambda_block=lambda_block: evaluate_structural(lambda_block, name, arguments)
    if expr_ast.names:
        expr_block = compile_expr(block, expr_ast.object)
        for name in expr_ast.names:
            if not callable(expr_block):
                raise Exception("Tried to access " + expr_ast.object + '.' + name + ", but it only has one output")
            expr_block = expr_block(name, None)
        return expr_block
    if expr_ast.callee:
        expr_block = compile_expr(block, expr_ast.callee)
        for call in expr_ast.calls:
            if not callable(expr_block):
                raise Exception("Tried to call " + expr_ast.callee + ", but it has no inputs")
            expr_block = expr_block(None, [compile_expr(block, a) for a in call])
        return expr_block
    raise Exception("Unknown expression " + expr_ast)

def iterate_subexpr(expr):
    for i in range(0, len(expr)):
        if expr[i] == '(':
            depth = 0
            for j in range(i, len(expr)):
                if expr[j] == '(':
                    depth += 1
                elif expr[j] == ')':
                    depth -= 1
                if depth == 0:
                    yield expr[i+1:j]
                    break

def optimize(expr):
    vars = []
    for e in reversed(list(iterate_subexpr(expr))):
        for i in range(0, len(vars)):
            e = e.replace(vars[i], 'a'+str(i))
        if (sum(v.count(e) for v in vars) + expr.count(e)) > 1 and len(e) > 10:
            vname = 'a' + str(len(vars))
            expr = expr.replace(e, vname)
            for i in range(0, len(vars)):
                vars[i] = vars[i].replace(e, vname)
            vars.append(e)
    return (expr, '\n'.join('    a'+str(i)+'='+vars[i] for i in range(0, len(vars))) + '\n')

def construct_output(fn):
    if isinstance(fn, str):
        return fn
    length = fn('count', None)
    if not length:
        raise Exception("Output is not a vector")
    try:
        length = int(length)
    except ValueError:
        raise Exception("output length is not constant")
    index = fn('index', None)
    elements = [construct_output(index(None, (str(i),))) for i in range(length)]
    return '(' + ','.join(elements) + ')'

def construct_array(var, array):
    if len(array) == 0:
        return var
    if array[-1] <= 1:
        return construct_array(var, array[:-1])
    args = [var+'['+str(n)+']' for n in range(array[-1])]
    return lambda name, new_args, args=args: evaluate_vector(args, name, new_args)

def construct_input(context, i, var):
    if i.array:
        return construct_array(var, [int(a) for a in i.array])
    if not i.type:
        raise Exception("Unknown type for "+i.name)
    ctor = context['drivers'][i.type]
    if not ctor:
        raise Exception("Couldn't find constructor for "+i.type)
    args = context['reflection'][i.type]['inputs']
    return ctor(None, [construct_input(context, args[n], var+'['+str(n)+']') for n in range(len(args))])

def compile(context, block_name, output_name):
    # Evaluate any constants
    for (_,block) in context['reflection'].items():
        if len(block['inputs']) == 0 and callable(context['drivers'][block['name']]):
            context['drivers'][block['name']] = context['drivers'][block['name']](None, None)
    block = context['drivers'][block_name]
    inputs = []
    input_names = []
    for i in context['reflection'][block_name]['inputs']:
        inputs.append(construct_input(context, i, i.name))
        input_names.append(i.name)
    body = construct_output(block(output_name, inputs))
    try:
        return eval(body, eval_context)
    except NameError:
        expr, vars = optimize(body)
        fcache = dict(eval_context)
        fnbody = 'def _expr(' + ','.join(input_names) + '):\n' + vars + '    return ' + expr
        print(fnbody)
        exec(fnbody, fcache)
        return fcache['_expr']

##############################################################################

def Random(n):
    A=53
    B=7175461
    C=11086637
    D=279470273
    E=2147483645
    na = n+A
    t1 = na**3 * B
    t2 = na**2 * C
    t3 = na * D
    t4 = t1 + t2 + t3
    return  (t4 % E) / E

context = new_context()
add_file(context, "test.ele")

compiled = compile(context, 'Random', 'return')

inputs = (123,)
import time
start = time.clock()
count = 1000000
for i in range(count):
    compiled(*inputs)
end = time.clock()
print((end - start) / count)

start = time.clock()
for i in range(count):
    Random(123)
end = time.clock()
print((end - start) / count)

compile(context, "汉字", 'return')

compile(context, "addC", 'return')