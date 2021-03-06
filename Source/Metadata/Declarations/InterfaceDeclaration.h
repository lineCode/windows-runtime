#pragma once

#include <guiddef.h>

#include "BaseClassDeclaration.h"

namespace NativeScript {
namespace Metadata {
    class InterfaceDeclaration : public BaseClassDeclaration {
    public:
        typedef BaseClassDeclaration Base;

        explicit InterfaceDeclaration(IMetaDataImport2*, mdTypeDef);

        virtual IID id() const;

    protected:
        explicit InterfaceDeclaration(DeclarationKind, IMetaDataImport2*, mdTypeDef);
    };

    class GenericInterfaceDeclaration final : public InterfaceDeclaration {
    public:
        typedef InterfaceDeclaration Base;

        explicit GenericInterfaceDeclaration(IMetaDataImport2*, mdTypeDef);

        size_t numberOfGenericParameters() const;
    };

    class GenericInterfaceInstanceDeclaration final : public InterfaceDeclaration {
    public:
        typedef InterfaceDeclaration Base;

        explicit GenericInterfaceInstanceDeclaration(IMetaDataImport2*, mdTypeDef, IMetaDataImport2*, mdTypeSpec);

        virtual std::wstring fullName() const override;

        virtual IID id() const override;

    private:
        const Microsoft::WRL::ComPtr<IMetaDataImport2> _closedMetadata;
        mdTypeSpec _closedToken;
    };
}
}
