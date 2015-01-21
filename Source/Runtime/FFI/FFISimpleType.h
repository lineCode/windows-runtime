#pragma once

#include "FFIType.h"

namespace NativeScript {
    template <typename T>
    class FFISimpleType : public JSC::JSNonFinalObject {
        typedef JSC::JSNonFinalObject Base;

        DECLARE_FFI_INFO

    public:
        static FFISimpleType<T>* create(JSC::VM& vm, JSC::Structure* structure, const ffi_type* ffiType) {
            ASSERT(ffiType);

            FFISimpleType* object = new (NotNull, JSC::allocateCell<FFISimpleType<T>>(vm.heap)) FFISimpleType<T>(vm, structure, ffiType);
            object->finishCreation(vm);
            return object;
        }

        static const unsigned StructureFlags = IsFFITypeFlag | Base::StructureFlags;

        static JSC::Structure* createStructure(JSC::VM& vm, JSC::JSGlobalObject* globalObject, JSC::JSValue prototype) {
            return JSC::Structure::create(vm, globalObject, prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), info());
        }

    private:
        FFISimpleType(JSC::VM& vm, JSC::Structure* structure, const ffi_type* ffiType)
            : Base(vm, structure), _ffiType(ffiType) {
        }

        static JSC::JSValue marshalNativeToJS(const JSC::JSCell*, JSC::ExecState* execState, const void* buffer) {
            return JSC::JSValue(*static_cast<const T*>(buffer));
        }

        static void marshalJSToNative(const JSC::JSCell*, JSC::ExecState* execState, JSC::JSValue value, void* buffer) {
            *reinterpret_cast<T*>(buffer) = value.toNumber(execState);
        }

        static const ffi_type* getFFITypeStruct(const JSCell* cell) {
            const FFISimpleType<T>* type = jsCast<const FFISimpleType<T>*>(cell);
            return type->_ffiType;
        }

        const ffi_type* _ffiType;
    };

    template <typename T>
    const FFITypeClassInfo FFISimpleType<T>::s_info = { "FFISimpleType", &Base::s_info, nullptr, CREATE_METHOD_TABLE(FFISimpleType), CREATE_FFI_METHOD_TABLE(FFISimpleType) };
}