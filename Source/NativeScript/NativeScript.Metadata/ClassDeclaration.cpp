#include "pch.h"
#include "ClassDeclaration.h"

namespace NativeScript {
namespace Metadata {

using namespace std;
using namespace Microsoft::WRL;

// TODO
namespace {

vector<MethodDeclaration> makeInitializerDeclarations(IMetaDataImport2* metadata, mdTypeDef token) {
    HCORENUM enumerator{nullptr};
    ULONG count{0};
    array<mdProperty, 1024> tokens;

    ASSERT_SUCCESS(metadata->EnumMethodsWithName(&enumerator, token, COR_CTOR_METHOD_NAME_W, tokens.data(), tokens.size(), &count));
    ASSERT(count < tokens.size() - 1);
    metadata->CloseEnum(enumerator);

    vector<MethodDeclaration> result;
    for (size_t i = 0; i < count; ++i) {
        result.emplace_back(metadata, tokens[i]);
    }

    return result;
}

vector<MethodDeclaration> makeMethodDeclarations(IMetaDataImport2* metadata, mdTypeDef token) {
    HCORENUM enumerator{nullptr};
    ULONG count{0};
    array<mdProperty, 1024> tokens;

    ASSERT_SUCCESS(metadata->EnumMethods(&enumerator, token, tokens.data(), tokens.size(), &count));
    ASSERT(count < tokens.size() - 1);
    metadata->CloseEnum(enumerator);

    vector<MethodDeclaration> result;
    for (size_t i = 0; i < count; ++i) {
        MethodDeclaration method{metadata, tokens[i]};

        if (!method.isExported()) {
            continue;
        }

        result.push_back(move(method));
    }

    return result;
}

vector<PropertyDeclaration> makePropertyDeclarations(IMetaDataImport2* metadata, mdTypeDef token) {
    HCORENUM enumerator{nullptr};
    ULONG count{0};
    array<mdMethodDef, 1024> tokens;

    ASSERT_SUCCESS(metadata->EnumProperties(&enumerator, token, tokens.data(), tokens.size(), &count));
    ASSERT(count < tokens.size() - 1);
    metadata->CloseEnum(enumerator);

    vector<PropertyDeclaration> result;
    for (size_t i = 0; i < count; ++i) {
        PropertyDeclaration property{metadata, tokens[i]};

        if (!property.isExported()) {
            continue;
        }

        result.push_back(move(property));
    }

    return result;
}

}

ClassDeclaration::ClassDeclaration(IMetaDataImport2* metadata, mdTypeDef token)
    : Base(metadata, token)
    , _initializers(makeInitializerDeclarations(metadata, token))
    , _methods(makeMethodDeclarations(metadata, token))
    , _properties(makePropertyDeclarations(metadata, token)) {

}

wstring ClassDeclaration::baseFullName() const {
    identifier parentName;
    ULONG parentNameLength;

    mdToken parentToken{mdTokenNil};
    ASSERT_SUCCESS(_metadata->GetTypeDefProps(_token, nullptr, 0, nullptr, nullptr, &parentToken));

    switch (TypeFromToken(parentToken)) {
        case mdtTypeDef: {
            ASSERT_SUCCESS(_metadata->GetTypeDefProps(parentToken, parentName.data(), parentName.size(), &parentNameLength, nullptr, nullptr));
            break;
        }

        case mdtTypeRef: {
            ASSERT_SUCCESS(_metadata->GetTypeRefProps(parentToken, nullptr, parentName.data(), parentName.size(), &parentNameLength));
            break;
        }

        default:
            ASSERT_NOT_REACHED();
    }

    wstring result{parentName.data(), parentNameLength - 1};
    return result;
}

// TODO: Abstract class
ClassType ClassDeclaration::classType() {
    HRESULT isComposable{_metadata->GetCustomAttributeByName(_token, COMPOSABLE_ATTRIBUTE_W, nullptr, nullptr)};
    ASSERT_SUCCESS(isComposable);
    if (isComposable == S_OK) {
        return ClassType::Subclassable;
    }

    HRESULT isInstantiable{_metadata->GetCustomAttributeByName(_token, ACTIVATABLE_ATTRIBUTE_W, nullptr, nullptr)};
    ASSERT_SUCCESS(isInstantiable);
    if (isInstantiable == S_OK) {
        return ClassType::Instantiable;
    }

    return ClassType::Uninstantiable;
}

IteratorRange<ClassDeclaration::MethodIterator> ClassDeclaration::initializers() const {
    return IteratorRange<MethodIterator>(_initializers.begin(), _initializers.end());
}

IteratorRange<ClassDeclaration::MethodIterator> ClassDeclaration::methods() const {
    return IteratorRange<MethodIterator>(_methods.begin(), _methods.end());
}

IteratorRange<ClassDeclaration::PropertyIterator> ClassDeclaration::properties() const {
    return IteratorRange<PropertyIterator>(_properties.begin(), _properties.end());
}

vector<unique_ptr<Declaration>> ClassDeclaration::findMembersWithName(const wchar_t* name) const {
    HCORENUM enumerator{nullptr};
    array<mdToken, 1024> memberTokens;
    ULONG membersSize{0};
    ASSERT_SUCCESS(_metadata->EnumMembersWithName(&enumerator, _token, name, memberTokens.data(), memberTokens.size(), &membersSize));
    _metadata->CloseEnum(enumerator);

    vector<unique_ptr<Declaration>> result;

    for (size_t i = 0; i < membersSize; ++i) {
        mdToken memberToken{memberTokens[i]};

        unique_ptr<Declaration> declaration;

        switch (TypeFromToken(memberToken)) {
            case mdtMethodDef:
                declaration = make_unique<MethodDeclaration>(_metadata.Get(), memberToken);
                break;

            case mdtProperty:
                declaration = make_unique<PropertyDeclaration>(_metadata.Get(), memberToken);
                break;

                // TODO: Add others

            default:
                ASSERT_NOT_REACHED();
        }

        if (!declaration->isExported()) {
            continue;
        }

        result.push_back(move(declaration));
    }

    return result;
}

vector<MethodDeclaration> ClassDeclaration::findMethodsWithName(const wchar_t* name) const {
    HCORENUM enumerator{nullptr};
    array<mdMethodDef, 1024> methodTokens;
    ULONG methodsCount{0};
    ASSERT_SUCCESS(_metadata->EnumMethodsWithName(&enumerator, _token, name, methodTokens.data(), methodTokens.size(), &methodsCount));
    _metadata->CloseEnum(enumerator);

    vector<MethodDeclaration> result;

    for (size_t i = 0; i < methodsCount; ++i) {
        mdMethodDef methodToken{methodTokens[i]};

        MethodDeclaration declaration{_metadata.Get(), methodToken};

        if (!declaration.isExported()) {
            continue;
        }

        result.push_back(declaration);
    }

    return result;
}

}
}
