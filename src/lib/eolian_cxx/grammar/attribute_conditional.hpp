#ifndef EOLIAN_CXX_ATTRIBUTE_CONDITIONAL_HH
#define EOLIAN_CXX_ATTRIBUTE_CONDITIONAL_HH

#include "grammar/generator.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename F, typename G>
struct functional_attribute_conditional_generator
{
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attribute, Context const& ctx) const
   {
     if(f(attribute))
       return as_generator(g).generate(sink, attribute, ctx);
     else
       return false;
   }

   F f;
   G g;
};

template <typename F, typename G>
struct is_eager_generator<functional_attribute_conditional_generator<F, G>> : std::true_type {};

template <typename F>
struct functional_attribute_conditional_directive
{
  template <typename G>
  functional_attribute_conditional_generator<F, G> operator[](G g) const
  {
    return {f, g};
  }

  template <typename OutputIterator, typename Attribute, typename Context>
  bool generate(OutputIterator, Attribute const& attribute, Context const&) const
  {
    return f(attribute);
  }
  
  F f;
};

template <typename F>
struct is_eager_generator<functional_attribute_conditional_directive<F>> : std::true_type {};
      
struct attribute_conditional_terminal
{
  template <typename F>
  functional_attribute_conditional_directive<F> operator()(F f) const
  {
    return {f};
  }
} const attribute_conditional = {};

namespace type_traits {
template <typename F, typename G>
struct attributes_needed<functional_attribute_conditional_generator<F, G>> : attributes_needed<G> {};  
template <typename F>
struct attributes_needed<functional_attribute_conditional_directive<F>> : std::integral_constant<int, 1> {};  
}
      
} } }

#endif
