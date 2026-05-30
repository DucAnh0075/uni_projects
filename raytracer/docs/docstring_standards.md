# Centralized Docstring Standards

## File Headers
```cpp
/**
 * @file filename.h
 * @brief Brief description of file purpose
 */
```

## Class Documentation
```cpp
/** @brief Brief class description
 *  @see docs/api_documentation.md for detailed documentation */
class ClassName {
```

## Function Documentation
```cpp
/** @brief Brief function description */
ReturnType functionName(parameters);
```

## Template for Detailed Function Documentation (in api_documentation.md)
```markdown
### FunctionName
**Purpose:** What the function does
**Location:** `filename.h`
**Parameters:**
- `param1` - Description of parameter 1
- `param2` - Description of parameter 2
**Returns:** Description of return value
**Algorithm:** Brief description of how it works
**Performance Impact:** Performance characteristics if relevant
```

## Template for Class Documentation (in api_documentation.md)
```markdown
### ClassName
**Purpose:** What the class does
**Location:** `filename.h`
**Description:** Detailed description
**Key Methods:**
- `method1()` - What it does
- `method2()` - What it does
**Performance Impact:** Performance characteristics
```

## Benefits of This Approach

1. **Minimal Code Clutter** - Headers stay clean and readable
2. **Centralized Updates** - Change documentation in one place
3. **Comprehensive Details** - Full documentation available in markdown
4. **Doxygen Compatible** - Still generates proper API docs
5. **Easy Maintenance** - No duplicated documentation to keep in sync
6. **Better Organization** - Related information grouped together
7. **Version Control Friendly** - Documentation changes don't clutter code diffs

## Usage Guidelines

### In Header Files
- Use brief, single-line docstrings for all public methods
- Reference centralized documentation with `@see docs/api_documentation.md`
- Keep file headers minimal with just `@file` and `@brief`

### In Centralized Documentation
- Provide comprehensive details, examples, and algorithms
- Include performance impact information
- Document usage patterns and common scenarios
- Maintain cross-references between related functions/classes

### For New Code
1. Add minimal docstring to header file
2. Add comprehensive documentation to `docs/api_documentation.md`
3. Update class/function lists in centralized docs
4. Add usage examples if the API is complex