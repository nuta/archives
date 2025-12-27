use quote::quote;
use syn::DeriveInput;
use syn::FieldsNamed;
use syn::FieldsUnnamed;

pub fn do_derive_error(input: DeriveInput) -> Result<proc_macro2::TokenStream, syn::Error> {
    let data_enum = match &input.data {
        syn::Data::Enum(data) => data,
        _ => {
            return Err(syn::Error::new_spanned(
                &input.ident,
                "Error can only be derived for enums",
            ));
        }
    };

    // Construct description str for each case: Error::Case(...) => "Case"
    let mut match_arms: Vec<_> = Vec::with_capacity(data_enum.variants.len());
    for variant in data_enum.variants.iter() {
        let variant_name = &variant.ident;
        let pattern = match &variant.fields {
            syn::Fields::Unit => quote! { Self::#variant_name },
            syn::Fields::Unnamed(FieldsUnnamed { unnamed, .. }) => {
                // Enumerate all fields to prevent "note: `InsertError` has a derived impl for the
                // trait `Debug`, but this is intentionally ignored during dead code analysis".
                let mut fields = Vec::with_capacity(unnamed.len());
                for (i, _) in unnamed.iter().enumerate() {
                    let field_name =
                        syn::Ident::new(&format!("_f{}", i), proc_macro2::Span::call_site());
                    fields.push(quote! { #field_name });
                }

                quote! { Self::#variant_name(#(#fields,)*) }
            }
            syn::Fields::Named(FieldsNamed { named, .. }) => {
                // Enumerate all fields to prevent "note: `InsertError` has a derived impl for the
                // trait `Debug`, but this is intentionally ignored during dead code analysis".
                let mut fields = Vec::with_capacity(named.len());
                for (i, _) in named.iter().enumerate() {
                    let field_name =
                        syn::Ident::new(&format!("_f{}", i), proc_macro2::Span::call_site());
                    fields.push(quote! { #field_name });
                }
                quote! { Self::#variant_name { #(#fields),* } }
            }
        };

        let variant_str = variant_name.to_string();
        match_arms.push(quote! {
            #pattern => #variant_str
        });
    }

    let enum_name = &input.ident;
    Ok(quote! {
        impl ::core::error::Error for #enum_name {
            fn source(&self) -> Option<&(dyn ::core::error::Error + 'static)> {
                None
            }

            fn description(&self) -> &str {
                match self {
                    #(#match_arms,)*
                }
            }

            fn cause(&self) -> Option<&dyn ::core::error::Error> {
                None
            }
        }

        impl ::core::fmt::Display for #enum_name {
            fn fmt(&self, f: &mut ::core::fmt::Formatter<'_>) -> ::core::fmt::Result {
                write!(f, "{:?}", self)
            }
        }
    })
}
