#ifndef TEST_H
#define TEST_H

#define TC(name)     void name(void)
#define RUN_TC(name) do { name(); printf("[OK] %s\n", #name); } while (0)

#endif
