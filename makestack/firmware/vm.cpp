#include <makestack/vm.h>

void vm_print_stacktrace(std::vector<Frame>& frames) {
    for (int i = frames.size() - 1; i >= 0; i--) {
        const SourceLoc& callee = frames[i].callee;
        vm_port_print("    %d: %s (%s:%d)\n",
            i, callee.func, callee.file, callee.lineno);
    }
}

void vm_print_error(ErrorInfo &info) {
    vm_port_print("Unhandled error: %s\n", info.message.c_str());
    vm_port_print("Backtrace:\n");
    vm_print_stacktrace(info.frames);
}

static void check_nargs_or_panic(int nargs, int nth) {
    if (nth >= nargs) {
        VM_PANIC("too few args");
    }
}

bool vm_get_bool_arg_or_panic(Context *ctx, int nargs, Value *args, int nth) {
    check_nargs_or_panic(nargs, nth);
    return args[nth].toBool();
}

int vm_get_int_arg_or_panic(Context *ctx, int nargs, Value *args, int nth) {
    check_nargs_or_panic(nargs, nth);
    return args[nth].toInt();
}

std::string vm_get_string_arg_or_panic(Context *ctx, int nargs, Value *args, int nth) {
    check_nargs_or_panic(nargs, nth);
    return args[nth].toString();
}

Value vm_get_arg_or_panic(Context *ctx, int nargs, Value *args, int nth) {
    check_nargs_or_panic(nargs, nth);
    return args[nth];
}

ErrorInfo::ErrorInfo(SourceLoc loc, std::string message) : message(message) {
    Context *ctx = vm_port_get_current_context();
    if (!ctx) {
        // FIXME: Called in the initialization phase.
        return;
    }

    frames = ctx->frames;
    frames.push_back(loc);
}

ErrorInfo::~ErrorInfo() {
    if (!checked) {
        vm_port_unhandled_error(*this);
    }
}

ValueInner *ValueInner::add(const ValueInner& rhs) {
    if (type == ValueType::String || rhs.type == ValueType::String) {
        std::string new_s = toString() + rhs.toString();
        return new ValueInner(new_s.c_str());
    } else if (type == ValueType::Int && rhs.type == ValueType::Int) {
        int new_i = toInt() + rhs.toInt();
        type = ValueType::Int;
        return new ValueInner(new_i);
    } else {
        /* Insane type combinations. */
        VM_PANIC("Invalid types for `+'.");
    }
}

ValueInner *ValueInner::sub(const ValueInner& rhs) {
    if (type == ValueType::Int && rhs.type == ValueType::Int) {
        int new_i = toInt() - rhs.toInt();
        return new ValueInner(new_i);
    } else {
        /* Insane type combinations. */
        VM_PANIC("Invalid types for `-'.");
    }
}

ValueInner *ValueInner::mul(const ValueInner& rhs) {
    if (type == ValueType::Int && rhs.type == ValueType::Int) {
        int new_i = toInt() * rhs.toInt();
        return new ValueInner(new_i);
    } else {
        /* Insane type combinations. */
        VM_PANIC("Invalid types for `*'.");
    }
}

ValueInner *ValueInner::div(const ValueInner& rhs) {
    if (type == ValueType::Int && rhs.type == ValueType::Int) {
        int new_i = toInt() / rhs.toInt();
        return new ValueInner(new_i);
    } else {
        /* Insane type combinations. */
        VM_PANIC("Invalid types for `/'.");
    }
}

ValueInner *ValueInner::mod(const ValueInner& rhs) {
    return new ValueInner(toInt() % rhs.toInt());
}

ValueInner *ValueInner::bitwise_and(const ValueInner& rhs) {
    return new ValueInner(toInt() & rhs.toInt());
}

ValueInner *ValueInner::bitwise_or(const ValueInner& rhs) {
    return new ValueInner(toInt() | rhs.toInt());
}

ValueInner *ValueInner::bitwise_xor(const ValueInner& rhs) {
    return new ValueInner(toInt() ^ rhs.toInt());
}

ValueInner *ValueInner::bitwise_lshift(const ValueInner& rhs) {
    return new ValueInner(toInt() << rhs.toInt());
}

ValueInner *ValueInner::bitwise_rshift(const ValueInner& rhs) {
    return new ValueInner(toInt() >> rhs.toInt());
}

ValueInner *ValueInner::bitwise_not() {
    return new ValueInner(~toInt());
}

ValueInner *ValueInner::unary_plus() {
    return new ValueInner(toInt());
}

ValueInner *ValueInner::unary_minus() {
    return new ValueInner(-toInt());
}


bool ValueInner::eq(const ValueInner& rhs) {
    if (type == rhs.type) {
        switch (type) {
        case ValueType::Int:
            return v_i == rhs.v_i;
        case ValueType::String:
            return v_s == rhs.v_s;
        case ValueType::Bool:
            return v_b == rhs.v_b;
        case ValueType::Null:
        case ValueType::Undefined:
            return true;
        default:
            VM_PANIC("Invalid types for `=='.");
        }
    } else {
        VM_PANIC("Invalid types for `=='.");
    }
}

bool ValueInner::gt(const ValueInner& rhs) {
    if (type == ValueType::Int && rhs.type == ValueType::Int) {
        return v_i > rhs.v_i;
    } else {
        VM_PANIC("Invalid types for `>'.");
    }
}

bool ValueInner::lt(const ValueInner& rhs) {
    if (type == ValueType::Int && rhs.type == ValueType::Int) {
        return v_i < rhs.v_i;
    } else {
        VM_PANIC("Invalid types for `<'.");
    }
}


void ValueInner::self_add(const ValueInner& rhs) {
    if (type == ValueType::String) {
        v_s += rhs.toString();
    } else if (type == ValueType::Int) {
        v_i += rhs.toInt();
    } else {
        VM_PANIC("Invalid types for `+='.");
    }
}

void ValueInner::self_sub(const ValueInner& rhs) {
    if (type != ValueType::Int) {
        VM_PANIC("Invalid types for `-='.");
    }

    v_i -= rhs.toInt();
}

void ValueInner::self_mul(const ValueInner& rhs) {
    if (type != ValueType::Int) {
        VM_PANIC("Invalid types for `*='.");
    }

    v_i *= rhs.toInt();
}

void ValueInner::self_div(const ValueInner& rhs) {
    if (type != ValueType::Int) {
        VM_PANIC("Invalid types for `/='.");
    }

    v_i /= rhs.toInt();

}

void ValueInner::self_mod(const ValueInner& rhs) {
    if (type != ValueType::Int) {
        VM_PANIC("Expected an integer.");
    }

    v_i = rhs.toInt();
}

void ValueInner::self_bitwise_and(const ValueInner& rhs) {
    if (type != ValueType::Int) {
        VM_PANIC("Expected an integer.");
    }

    v_i &= rhs.toInt();
}

void ValueInner::self_bitwise_or(const ValueInner& rhs) {
    if (type != ValueType::Int) {
        VM_PANIC("Expected an integer.");
    }

    v_i |= rhs.toInt();
}

void ValueInner::self_bitwise_xor(const ValueInner& rhs) {
    if (type != ValueType::Int) {
        VM_PANIC("Expected an integer.");
    }

    v_i ^= rhs.toInt();
}

void ValueInner::self_bitwise_lshift(const ValueInner& rhs) {
    if (type != ValueType::Int) {
        VM_PANIC("Expected an integer.");
    }

    v_i <<= rhs.toInt();
}

void ValueInner::self_bitwise_rshift(const ValueInner& rhs) {
    if (type != ValueType::Int) {
        VM_PANIC("Expected an integer.");
    }

    v_i >>= rhs.toInt();
}



Value Value::call(Context *ctx, int nargs, Value *args) {
    if (inner->type != ValueType::Function) {
        return VM_CREATE_ERROR("not callable");
    }

    return inner->v_f(ctx, nargs, args);
}

Value Value::get(Value prop) {
    switch (inner->type) {
    case ValueType::Object: {
        if (prop.type() != ValueType::String) {
            return VM_CREATE_ERROR("prop must be string");
        }

        std::string propString = prop.toString();
        if (inner->v_obj.find(propString) == inner->v_obj.end()) {
            return Value::Undefined();
        }

        ValueInner *inner_value = inner->v_obj[propString];
        inner_value->ref_count++;
        return Value(inner_value);
    }
    default:
        return Value::Undefined();
    }
}

Value Value::set(Value prop, Value value) {
    switch (inner->type) {
    case ValueType::Object: {
        if (prop.type() != ValueType::String) {
            return VM_CREATE_ERROR("prop must be string");
        }

        value.inner->ref_count++;
        inner->v_obj[prop.toString()] = value.inner;
        return value;
    }
    default:
        return Value::Undefined();
    }
}

Value Scope::get(const char *id) {
    Scope *scope = this;
    while (scope) {
        if (scope->vars.find(id) != scope->vars.end()) {
            return scope->vars[id].value;
        }

        scope = scope->prev;
    }

    VM_PANIC("undefined reference: %s", id);
}

Value Scope::set(const char *id, Value value) {
    vars[id] = Var(value);
    return value;
}

void Context::enter_scope(SourceLoc callee) {
    Scope *new_scope = new Scope(current);
    current = new_scope;
    frames.push_back(callee);
}

void Context::leave_scope() {
    VM_ASSERT(current != nullptr);
    Scope *prev = current->prev;
    current->ref_count--;
    if (current->ref_count == 0) {
        delete current;
    }
    current = prev;
    frames.pop_back();
}

Scope *Context::create_closure_scope() {
    current->ref_count++;
    return current;
}

Value Context::call(SourceLoc called_from, Value func, int nargs, Value *args) {
    enter_scope(called_from);
    Value ret = func.call(this, nargs, args);
    leave_scope();
    return ret;
}
