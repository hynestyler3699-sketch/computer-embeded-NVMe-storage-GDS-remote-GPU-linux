# Contributing to ComputerVisionEmbedded

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## 🚀 Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:

   ```bash
   git clone https://github.com/YOUR_USERNAME/computer-embeded-NVMe-storage-GDS-remote-GPU-linux.git
   cd computer-embeded-NVMe-storage-GDS-remote-GPU-linux
   ```

3. **Add upstream remote**:

   ```bash
   git remote add upstream https://github.com/hynestyler3699-sketch/computer-embeded-NVMe-storage-GDS-remote-GPU-linux.git
   ```

## 📝 Development Workflow

### Creating a Branch

```bash
# Update main branch
git checkout main
git pull upstream main

# Create feature branch
git checkout -b feature/your-feature-name
```

### Making Changes

1. Write your code following the style guidelines
2. Add tests for new functionality
3. Update documentation as needed
4. Commit with descriptive messages

### Commit Messages

Use clear, descriptive commit messages:

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

Types:

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `style`: Code style (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding tests
- `perf`: Performance improvement
- `chore`: Maintenance tasks

Examples:

```
feat(cv): add Canny edge detection algorithm
fix(gds): resolve memory leak in loader cleanup
docs(readme): update installation instructions
```

### Submitting a Pull Request

1. Push your branch:

   ```bash
   git push origin feature/your-feature-name
   ```

2. Open a Pull Request on GitHub
3. Fill out the PR template
4. Wait for review and address feedback

## 🎨 Code Style

### C/C++ Guidelines

- Use `.clang-format` for automatic formatting
- 2-space indentation
- 100 character line limit
- Include guards for headers: `#ifndef HEADER_H`
- Document functions with Doxygen-style comments

```c
/**
 * @brief Brief description of function
 * @param param1 Description of parameter
 * @return Description of return value
 */
int function_name(int param1);
```

### Python Guidelines

- Follow PEP 8
- Use type hints
- Document with docstrings

## 🧪 Testing

### Running Tests

```bash
# Build with tests
cmake -B build -DBUILD_TESTS=ON
cmake --build build

# Run tests
cd build
ctest --output-on-failure
```

### Writing Tests

- Place tests in `tests/unit_tests/` or `tests/integration_tests/`
- Name test files `test_<component>.c`
- Cover edge cases and error conditions

## 📚 Documentation

Update documentation when:

- Adding new features
- Changing APIs
- Fixing user-facing bugs

Documentation files:

- `README.md` - Project overview
- `docs/` - Detailed documentation
- Code comments - Implementation details

## 🐛 Reporting Issues

Use [GitHub Issues](https://github.com/hynestyler3699-sketch/computer-embeded-NVMe-storage-GDS-remote-GPU-linux/issues) with appropriate templates:

- **Bug reports**: Include steps to reproduce, expected vs. actual behavior
- **Feature requests**: Describe the use case and proposed solution
- **Performance issues**: Include benchmarks and environment details

## ✅ Checklist

Before submitting a PR, ensure:

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] Documentation is updated
- [ ] Commit messages are clear
- [ ] PR description explains the changes
- [ ] Related issues are linked

## 🤝 Code of Conduct

Be respectful and constructive in all interactions. We're all here to learn and improve.

## 📞 Questions?

Feel free to open an issue for questions or discussions!
