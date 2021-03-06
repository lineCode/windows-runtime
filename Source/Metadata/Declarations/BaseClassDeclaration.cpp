#include "Metadata-Prefix.h"
#include "BaseClassDeclaration.h"
#include "InterfaceDeclaration.h"
#include "DeclarationFactory.h"

namespace NativeScript {
namespace Metadata {

    using namespace std;
    using namespace Microsoft::WRL;

    // TODO
    namespace {

        vector<unique_ptr<const InterfaceDeclaration>> makeImplementedInterfacesDeclarations(IMetaDataImport2* metadata, mdTypeDef token) {
            HCORENUM enumerator{ nullptr };
            ULONG count{ 0 };
            array<mdInterfaceImpl, 1024> tokens;

            ASSERT_SUCCESS(metadata->EnumInterfaceImpls(&enumerator, token, tokens.data(), tokens.size(), &count));
            ASSERT(count < tokens.size() - 1);
            metadata->CloseEnum(enumerator);

            vector<unique_ptr<const InterfaceDeclaration>> result;
            for (size_t i = 0; i < count; ++i) {
                mdToken interfaceToken{ mdTokenNil };
                ASSERT_SUCCESS(metadata->GetInterfaceImplProps(tokens[i], nullptr, &interfaceToken));

                result.push_back(DeclarationFactory::makeInterfaceDeclaration(metadata, interfaceToken));
            }

            return result;
        }

        vector<MethodDeclaration> makeMethodDeclarations(IMetaDataImport2* metadata, mdTypeDef token) {
            HCORENUM enumerator{ nullptr };
            ULONG count{ 0 };
            array<mdProperty, 1024> tokens;

            ASSERT_SUCCESS(metadata->EnumMethods(&enumerator, token, tokens.data(), tokens.size(), &count));
            ASSERT(count < tokens.size() - 1);
            metadata->CloseEnum(enumerator);

            vector<MethodDeclaration> result;
            for (size_t i = 0; i < count; ++i) {
                MethodDeclaration method{ metadata, tokens[i] };

                if (!method.isExported()) {
                    continue;
                }

                result.push_back(move(method));
            }

            return result;
        }

        vector<PropertyDeclaration> makePropertyDeclarations(IMetaDataImport2* metadata, mdTypeDef token) {
            HCORENUM enumerator{ nullptr };
            ULONG count{ 0 };
            array<mdMethodDef, 1024> tokens;

            ASSERT_SUCCESS(metadata->EnumProperties(&enumerator, token, tokens.data(), tokens.size(), &count));
            ASSERT(count < tokens.size() - 1);
            metadata->CloseEnum(enumerator);

            vector<PropertyDeclaration> result;
            for (size_t i = 0; i < count; ++i) {
                PropertyDeclaration property{ metadata, tokens[i] };

                if (!property.isExported()) {
                    continue;
                }

                result.push_back(move(property));
            }

            return result;
        }

        vector<EventDeclaration> makeEventDeclarations(IMetaDataImport2* metadata, mdTypeDef token) {
            HCORENUM enumerator{ nullptr };
            ULONG count{ 0 };
            array<mdEvent, 1024> tokens;

            ASSERT_SUCCESS(metadata->EnumEvents(&enumerator, token, tokens.data(), tokens.size(), &count));
            ASSERT(count < tokens.size() - 1);
            metadata->CloseEnum(enumerator);

            vector<EventDeclaration> result;
            for (size_t i = 0; i < count; ++i) {
                EventDeclaration event{ metadata, tokens[i] };

                if (!event.isExported()) {
                    continue;
                }

                result.push_back(move(event));
            }

            return result;
        }
    }

    BaseClassDeclaration::BaseClassDeclaration(DeclarationKind kind, IMetaDataImport2* metadata, mdTypeDef token)
        : Base(kind, metadata, token)
        , _implementedInterfaces(makeImplementedInterfacesDeclarations(metadata, token))
        , _methods(makeMethodDeclarations(metadata, token))
        , _properties(makePropertyDeclarations(metadata, token))
        , _events(makeEventDeclarations(metadata, token)) {
    }

    IteratorRange<BaseClassDeclaration::InterfaceIterator> BaseClassDeclaration::implementedInterfaces() const {
        return IteratorRange<InterfaceIterator>(_implementedInterfaces.begin(), _implementedInterfaces.end());
    }

    IteratorRange<BaseClassDeclaration::MethodIterator> BaseClassDeclaration::methods() const {
        return IteratorRange<MethodIterator>(_methods.begin(), _methods.end());
    }

    IteratorRange<BaseClassDeclaration::PropertyIterator> BaseClassDeclaration::properties() const {
        return IteratorRange<PropertyIterator>(_properties.begin(), _properties.end());
    }

    IteratorRange<BaseClassDeclaration::EventIterator> BaseClassDeclaration::events() const {
        return IteratorRange<EventIterator>(_events.begin(), _events.end());
    }

    vector<unique_ptr<const Declaration>> BaseClassDeclaration::findMembersWithName(const wchar_t* name) const {
        ASSERT(name);

        vector<unique_ptr<const Declaration>> result;

        for (const MethodDeclaration& method : findMethodsWithName(name)) {
            result.push_back(make_unique<MethodDeclaration>(method));
        }

        for (const PropertyDeclaration& property : _properties) {
            if (property.fullName() == name) {
                result.push_back(make_unique<PropertyDeclaration>(property));
            }
        }

        for (const EventDeclaration& event : _events) {
            if (event.fullName() == name) {
                result.push_back(make_unique<EventDeclaration>(event));
            }
        }

        return result;
    }

    vector<MethodDeclaration> BaseClassDeclaration::findMethodsWithName(const wchar_t* name) const {
        ASSERT(name);

        HCORENUM enumerator{ nullptr };
        array<mdMethodDef, 1024> methodTokens;
        ULONG methodsCount{ 0 };
        ASSERT_SUCCESS(_metadata->EnumMethodsWithName(&enumerator, _token, name, methodTokens.data(), methodTokens.size(), &methodsCount));
        _metadata->CloseEnum(enumerator);

        vector<MethodDeclaration> result;

        for (size_t i = 0; i < methodsCount; ++i) {
            mdMethodDef methodToken{ methodTokens[i] };
            result.emplace_back(_metadata.Get(), methodToken);
        }

        return result;
    }
}
}
