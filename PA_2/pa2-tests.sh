#!/usr/bin/env bash
# ============================================================
# PA2 Aggie Shell — Rubric-Aligned Tests (v4.2)
# ============================================================

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

SCORE=0
MAX_SCORE=100

remake() { make clean >/dev/null 2>&1; make -s >/dev/null 2>&1; }

echo -e "${YELLOW}Starting Aggie Shell rubric-aligned tests...${NC}\n"
remake

# ============================================================
# 1. Echo (5 pts)
# ============================================================
echo "[Test 1] echo"
if ./shell <<< 'echo "Hello world | Life is Good > Great $"; exit' | grep -q "Hello world"; then
  echo -e "  ${GREEN}Passed${NC}"
  SCORE=$((SCORE+5))
else
  echo -e "  ${RED}Failed${NC}"
fi
echo "SCORE: ${SCORE}/${MAX_SCORE}\n"

# ============================================================
# 2. Simple Commands (10 pts)
# ============================================================
echo "[Test 2] simple commands with arguments"
if ./shell <<< "ls && ls -l /usr/bin && ls -la && ps aux && exit" >/dev/null 2>&1; then
  echo -e "  ${GREEN}Passed${NC}"
  SCORE=$((SCORE+10))
else
  echo -e "  ${RED}Failed${NC}"
fi
echo "SCORE: ${SCORE}/${MAX_SCORE}\n"

# ============================================================
# 3. Input/Output Redirection (15 pts)
# ============================================================
echo "[Test 3] input/output redirection"
./shell <<< "ps aux > a && grep /init < a && grep /init < a > b && exit" >/dev/null 2>&1
if [[ -s a && -s b ]]; then
  echo -e "  ${GREEN}Passed${NC}"
  SCORE=$((SCORE+15))
else
  echo -e "  ${RED}Failed${NC}"
fi
rm -f a b
echo "SCORE: ${SCORE}/${MAX_SCORE}\n"

# ============================================================
# 4. Single Pipe (8 pts)
# ============================================================
echo "[Test 4] single pipe"
if ./shell <<< "ls -l | grep shell && exit" | grep -q "shell"; then
  echo -e "  ${GREEN}Passed${NC}"
  SCORE=$((SCORE+8))
else
  echo -e "  ${RED}Failed${NC}"
fi
echo "SCORE: ${SCORE}/${MAX_SCORE}\n"

# ============================================================
# 5. Two or More Pipes (6 pts)
# ============================================================
echo "[Test 5] multiple pipes"
if ./shell <<< "ps aux | awk '/usr/{print \$1}' | sort -r | head -n 1 && exit" >/dev/null 2>&1; then
  echo -e "  ${GREEN}Passed${NC}"
  SCORE=$((SCORE+6))
else
  echo -e "  ${RED}Failed${NC}"
fi
echo "SCORE: ${SCORE}/${MAX_SCORE}\n"

# ============================================================
# 6. Two or More Pipes w/ I/O Redirection (15 pts)
# ============================================================
echo "[Test 6] multiple pipes + I/O redirection"
./shell <<< "ps aux > test.txt && awk '{print \$1, \$11}' < test.txt | head -10 | tr a-z A-Z | sort > output.txt && cat output.txt && exit" >/dev/null 2>&1
if [[ -s test.txt && -s output.txt ]]; then
  echo -e "  ${GREEN}Passed${NC}"
  SCORE=$((SCORE+15))
else
  echo -e "  ${RED}Failed${NC}"
fi
rm -f test.txt output.txt
echo "SCORE: ${SCORE}/${MAX_SCORE}\n"

# ============================================================
# 7. Background Processes (5 pts)
# ============================================================
echo "[Test 7] background processes"
START=$(date +%s)
./shell <<< "sleep 3 &; sleep 2; exit" >/dev/null 2>&1
END=$(date +%s)
if (( END - START < 3 )); then
  echo -e "  ${GREEN}Passed${NC} (didn't block)"
  SCORE=$((SCORE+5))
else
  echo -e "  ${RED}Failed${NC}"
fi
echo "SCORE: ${SCORE}/${MAX_SCORE}\n"

# ============================================================
# 8. Directory Processing (6 pts)
# ============================================================
echo "[Test 8] cd command"
TMPDIR=$(pwd)
OUT=$(./shell <<< "cd ../../ && pwd && cd - && exit" 2>/dev/null)
if echo "$OUT" | grep -q "$TMPDIR"; then
  echo -e "  ${GREEN}Passed${NC}"
  SCORE=$((SCORE+6))
else
  echo -e "  ${RED}Failed${NC}"
fi
echo "SCORE: ${SCORE}/${MAX_SCORE}\n"

# ============================================================
# 9. User Prompt (5 pts)
# ============================================================
echo "[Test 9] user prompt"
PROMPT_OUT=$(echo "exit" | ./shell 2>/dev/null)
if echo "$PROMPT_OUT" | grep -Eq "$(whoami).*$(date +%b)"; then
  echo -e "  ${GREEN}Passed${NC}"
  SCORE=$((SCORE+5))
else
  echo -e "  ${RED}Failed${NC}"
fi
echo "SCORE: ${SCORE}/${MAX_SCORE}\n"


# ============================================================
# Final Score
# ============================================================
echo -e "\n${YELLOW}Final SCORE: ${SCORE}/${MAX_SCORE}${NC}\n"
