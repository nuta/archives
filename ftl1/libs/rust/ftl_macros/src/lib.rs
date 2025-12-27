extern crate proc_macro;

mod error;

#[proc_macro_derive(Error)]
pub fn derive_error(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    let input = syn::parse_macro_input!(input as syn::DeriveInput);
    error::do_derive_error(input)
        .unwrap_or_else(|e| e.into_compile_error())
        .into()
}
