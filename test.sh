#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "🧪 Running BirdSharp Tests..."
echo ""

mkdir -p tmp

TESTS_PASSED=0
TESTS_FAILED=0

echo -n "Test 1: Basic compilation (code/main.bsh)... "
bs code/main.bsh -o tmp/test_main 2>/dev/null
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((TESTS_FAILED++))
fi

echo -n "Test 2: IR generation... "
if [ -f tmp/test_main.ir ]; then
    if grep -q "%func" tmp/test_main.ir; then
        echo -e "${GREEN}✓ PASS${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}✗ FAIL (invalid IR)${NC}"
        ((TESTS_FAILED++))
    fi
else
    echo -e "${RED}✗ FAIL (no IR file)${NC}"
    ((TESTS_FAILED++))
fi

echo ""
echo -e "${YELLOW}Testing all examples in lang/examples/...${NC}"
echo ""

TEST_NUM=3
for example in lang/examples/*.bsh; do
    if [ -f "$example" ]; then
        filename=$(basename "$example" .bsh)
        echo -n "Test $TEST_NUM: $filename... "
        
        # Compile the example
        bs "$example" -o "tmp/test_$filename" 2>/dev/null
        
        if [ $? -eq 0 ]; then
            # Check if executable was created
            if [ -f "tmp/test_$filename" ]; then
                echo -e "${GREEN}✓ PASS (compiled)${NC}"
                ((TESTS_PASSED++))
            else
                echo -e "${RED}✗ FAIL (no executable)${NC}"
                ((TESTS_FAILED++))
            fi
        else
            echo -e "${RED}✗ FAIL (compilation error)${NC}"
            ((TESTS_FAILED++))
        fi
        
        ((TEST_NUM++))
    fi
done

echo ""
echo "================================"
echo -e "Tests passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests failed: ${RED}$TESTS_FAILED${NC}"
echo "================================"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed! 🎉${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed! ❌${NC}"
    exit 1
fi
