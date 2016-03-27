namespace {{api}}
{


GLBINDING_API bool operator==(const {{identifier}} & a, std::underlying_type<{{identifier}}>::type b);
GLBINDING_API bool operator!=(const {{identifier}} & a, std::underlying_type<{{identifier}}>::type b);
GLBINDING_API bool operator< (const {{identifier}} & a, std::underlying_type<{{identifier}}>::type b);
GLBINDING_API bool operator<=(const {{identifier}} & a, std::underlying_type<{{identifier}}>::type b);
GLBINDING_API bool operator> (const {{identifier}} & a, std::underlying_type<{{identifier}}>::type b);
GLBINDING_API bool operator>=(const {{identifier}} & a, std::underlying_type<{{identifier}}>::type b);

GLBINDING_API bool operator==(std::underlying_type<{{identifier}}>::type a, const {{identifier}} & b);
GLBINDING_API bool operator!=(std::underlying_type<{{identifier}}>::type a, const {{identifier}} & b);
GLBINDING_API bool operator< (std::underlying_type<{{identifier}}>::type a, const {{identifier}} & b);
GLBINDING_API bool operator<=(std::underlying_type<{{identifier}}>::type a, const {{identifier}} & b);
GLBINDING_API bool operator> (std::underlying_type<{{identifier}}>::type a, const {{identifier}} & b);
GLBINDING_API bool operator>=(std::underlying_type<{{identifier}}>::type a, const {{identifier}} & b);


} // namespace {{api}}
