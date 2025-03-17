# Bomb Lab
这个实验我是用Docker做的，没有用到虚拟机。
其中用了Ubuntu的镜像，将实验目录挂载到了容器中。
 
## Phase 1
首先：

`objdump -d bomb > bomb.txt`
```
0000000000400ee0 <phase_1>:
  400ee0:	48 83 ec 08          	sub    $0x8,%rsp
  400ee4:	be 00 24 40 00       	mov    $0x402400,%esi
  400ee9:	e8 4a 04 00 00       	call   401338 <strings_not_equal>
  400eee:	85 c0                	test   %eax,%eax
  400ef0:	74 05                	je     400ef7 <phase_1+0x17>
  400ef2:	e8 43 05 00 00       	call   40143a <explode_bomb>
  400ef7:	48 83 c4 08          	add    $0x8,%rsp
  400efb:	c3                   	ret
```

gdb中，运行：
`break *0400ee0`
`x/s 0x402400` (x是“examine”，指定格式字符串“s”)
得到：
`0x402400:	"Border relations with Canada have never been better."`
说明这是作为第二个参数传到strings_not_equal这个函数中的。答案就是句话了。
 
## Phase 2
```
0000000000400efc <phase_2>:
  400efc:	55                   	push   %rbp
  400efd:	53                   	push   %rbx
  400efe:	48 83 ec 28          	sub    $0x28,%rsp                     /* 留出一个40字节的位置 */
  400f02:	48 89 e6             	mov    %rsp,%rsi                      /*6个数字传入read_six_numbers*/
  400f05:	e8 52 05 00 00       	call   40145c <read_six_numbers>
  400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)                    /* 第一个数字是必须1 */
  400f0e:	74 20                	je     400f30 <phase_2+0x34>
  400f10:	e8 25 05 00 00       	call   40143a <explode_bomb>
  400f15:	eb 19                	jmp    400f30 <phase_2+0x34>
  400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax
  400f1a:	01 c0                	add    %eax,%eax                      /* eax *= 2 */
  400f1c:	39 03                	cmp    %eax,(%rbx)                    /* rbx == (eax *= 2)? */
  400f1e:	74 05                	je     400f25 <phase_2+0x29>
  400f20:	e8 15 05 00 00       	call   40143a <explode_bomb>
  400f25:	48 83 c3 04          	add    $0x4,%rbx                      /* 到没到栈底？*/
  400f29:	48 39 eb             	cmp    %rbp,%rbx
  400f2c:	75 e9                	jne    400f17 <phase_2+0x1b>
  400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40>
  400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx                 /* rbx = &[rsp+4] */
  400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp                /* rbp指向rsp + 24字节，6个整数 */
  400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>
  400f3c:	48 83 c4 28          	add    $0x28,%rsp
  400f40:	5b                   	pop    %rbx
  400f41:	5d                   	pop    %rbp
  400f42:	c3                   	ret
```
无需设置断点来分析。我给出`1 2 4 8 16 32`。

## Phase 3

越来越长啦！这题用了switch语句。
```
0000000000400f43 <phase_3>:
  400f43:	48 83 ec 18          	sub    $0x18,%rsp                     /* 栈留出24字节 */
  400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx                 /* rcx = &[rsp + 12] */
  400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx                 /* rdx = &[rsp + 8] */
  400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi                 /* esi = 0x4025cf */
  400f56:	b8 00 00 00 00       	mov    $0x0,%eax
  400f5b:	e8 90 fc ff ff       	call   400bf0 <__isoc99_sscanf@plt>   /* sscanf */
  400f60:	83 f8 01             	cmp    $0x1,%eax                      /* sscanf要成功解析，所以至少要返回1 */
  400f63:	7f 05                	jg     400f6a <phase_3+0x27>
  400f65:	e8 d0 04 00 00       	call   40143a <explode_bomb>
  400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp)                 /* 如果 [rsp + 8] > 7 爆炸 */
  400f6f:	77 3c                	ja     400fad <phase_3+0x6a> 
  400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax                 /* 比较sscanf的第一个解析参数 */
  400f75:	ff 24 c5 70 24 40 00 	jmp    *0x402470(,%rax,8)             /*switch 0x402470 + rax * 8 */
  400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax 
  400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
  400f83:	b8 c3 02 00 00       	mov    $0x2c3,%eax
  400f88:	eb 34                	jmp    400fbe <phase_3+0x7b>
  400f8a:	b8 00 01 00 00       	mov    $0x100,%eax
  400f8f:	eb 2d                	jmp    400fbe <phase_3+0x7b>
  400f91:	b8 85 01 00 00       	mov    $0x185,%eax
  400f96:	eb 26                	jmp    400fbe <phase_3+0x7b>
  400f98:	b8 ce 00 00 00       	mov    $0xce,%eax
  400f9d:	eb 1f                	jmp    400fbe <phase_3+0x7b>
  400f9f:	b8 aa 02 00 00       	mov    $0x2aa,%eax
  400fa4:	eb 18                	jmp    400fbe <phase_3+0x7b>
  400fa6:	b8 47 01 00 00       	mov    $0x147,%eax
  400fab:	eb 11                	jmp    400fbe <phase_3+0x7b>
  400fad:	e8 88 04 00 00       	call   40143a <explode_bomb>
  400fb2:	b8 00 00 00 00       	mov    $0x0,%eax
  400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
  400fb9:	b8 37 01 00 00       	mov    $0x137,%eax                  /* f75直接跳到这里，eax = 0x137 */
  400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax               /* ecx = eax? */
  400fc2:	74 05                	je     400fc9 <phase_3+0x86>
  400fc4:	e8 71 04 00 00       	call   40143a <explode_bomb>
  400fc9:	48 83 c4 18          	add    $0x18,%rsp
  400fcd:	c3                   	ret
```

观察来，好像栈就是会预留16个空字节。数组的大小可以通过（rsp-16）/4得到。
看来代码用到了sscanf函数，函数原型是
`int sscanf(const char *str, const char *format, ...);`
[参考](https://www.tutorialspoint.com/c_standard_library/c_function_sscanf.htm)说明如下：
**str**: The input string from which to read.
**format**: A format string that specifies how to interpret the input string.
**...**: Additional arguments pointing to variables where the extracted values will be stored.
sscanf的返回值是成功解析的值的个数。
看起来传给esi的是格式参数。但是是什么其实不重要，因为通过后面rdx和rcx的预留位置可以推测，是两个四字节的参数，所以我猜测是整型。在400f60设置了断点。
`break *0x400f60`
检查：
`x/wx $rps+8`
`x/wx $rps+12`
和推测得一样，给sscanf解析出的结果是预留的位置。
下面用到了switch语句的跳转表，而且获悉如果 [rsp+8] > 7会爆炸，可以推断有6条语句，且 [rcx] < 7。我尝试了 1 1。
发现运行到0x400f75会直接跳转到0x400fb9。后面很好读，就是比较sccanf第二个解析出来的参数是不是和eax中的一样（0x137）。最终答案是 `1 311`。这题是多解的。
 
## Phase 4
```
0000000000400fce <func4>:
  400fce:	48 83 ec 08          	sub    $0x8,%rsp                    /* 预留8字节给栈 */
  400fd2:	89 d0                	mov    %edx,%eax                    /* eax = 14 */
  400fd4:	29 f0                	sub    %esi,%eax                    /* eax -= esi (esi是0) */
  400fd6:	89 c1                	mov    %eax,%ecx                    /* ecx = 14 */
  400fd8:	c1 e9 1f             	shr    $0x1f,%ecx                   /* ecx右移31位 */
  400fdb:	01 c8                	add    %ecx,%eax                    /* eax += ecx */
  400fdd:	d1 f8                	sar    $1,%eax                      /* eax 算数右移1位 */
  400fdf:	8d 0c 30             	lea    (%rax,%rsi,1),%ecx           /* ecx = rax + rsi */
  400fe2:	39 f9                	cmp    %edi,%ecx                    /* 14 >= edi? 是的话返回值等于0*/
  400fe4:	7e 0c                	jle    400ff2 <func4+0x24>
  400fe6:	8d 51 ff             	lea    -0x1(%rcx),%edx
  400fe9:	e8 e0 ff ff ff       	call   400fce <func4>
  400fee:	01 c0                	add    %eax,%eax
  400ff0:	eb 15                	jmp    401007 <func4+0x39>
  400ff2:	b8 00 00 00 00       	mov    $0x0,%eax                   /* eax = 0 */		   
  400ff7:	39 f9                	cmp    %edi,%ecx                   /* ecx >= edi? 如果是的话就返回了 */
  400ff9:	7d 0c                	jge    401007 <func4+0x39>
  400ffb:	8d 71 01             	lea    0x1(%rcx),%esi
  400ffe:	e8 cb ff ff ff       	call   400fce <func4>
  401003:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax
  401007:	48 83 c4 08          	add    $0x8,%rsp
  40100b:	c3                   	ret

000000000040100c <phase_4>:
  40100c:	48 83 ec 18          	sub    $0x18,%rsp                 /* 栈预留24字节 */
  401010:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx             /* rcx = &[rsp + 12] */
  401015:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx             /* rdx = &[rsp + 8] */
  40101a:	be cf 25 40 00       	mov    $0x4025cf,%esi             /* 同上题，格式参数 */
  40101f:	b8 00 00 00 00       	mov    $0x0,%eax
  401024:	e8 c7 fb ff ff       	call   400bf0 <__isoc99_sscanf@plt>
  401029:	83 f8 02             	cmp    $0x2,%eax                  /* 如果eax = 2，那就是读两个参数嘛 */
  40102c:	75 07                	jne    401035 <phase_4+0x29>
  40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp)             /* [rsp + 8] < 0xe? 是的话就过*/
  401033:	76 05                	jbe    40103a <phase_4+0x2e>
  401035:	e8 00 04 00 00       	call   40143a <explode_bomb>
  40103a:	ba 0e 00 00 00       	mov    $0xe,%edx                  /* func4 第三个参数是0xe */
  40103f:	be 00 00 00 00       	mov    $0x0,%esi                  /* 第二个参数是0 */
  401044:	8b 7c 24 08          	mov    0x8(%rsp),%edi             /* 第一个参数是[rsp + 8] */
  401048:	e8 81 ff ff ff       	call   400fce <func4>		     
  40104d:	85 c0                	test   %eax,%eax
  40104f:	75 07                	jne    401058 <phase_4+0x4c>      /* 返回非0值，爆炸 */
  401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp)             /* [rsp + 12] = 0？是的话就过了*/
  401056:	74 05                	je     40105d <phase_4+0x51>
  401058:	e8 dd 03 00 00       	call   40143a <explode_bomb>
  40105d:	48 83 c4 18          	add    $0x18,%rsp
  401061:	c3                   	ret
```

这个和上题有点类似。也就是sscanf解析的第一个值（rdi中的值）必须在[0,14]这个区间中，否则满足不了func4的逻辑。
sscanf的第二个值很直接，就是0。
看起来func4是递归定义的。我这里答案写的 `1 0`。

## Phase 5
```
0000000000401062 <phase_5>:
  401062:	53                   	push   %rbx
  401063:	48 83 ec 20          	sub    $0x20,%rsp                 /* 栈留了32个字节 */
  401067:	48 89 fb             	mov    %rdi,%rbx
  40106a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax              /* 从 fs 段的 0x28 偏移处读取数据 */
  401071:	00 00 
  401073:	48 89 44 24 18       	mov    %rax,0x18(%rsp)            /* [rsp + 18] = rax */
  401078:	31 c0                	xor    %eax,%eax
  40107a:	e8 9c 02 00 00       	call   40131b <string_length>     /* 参数是rbx处的值，或许是输入值 */
  40107f:	83 f8 06             	cmp    $0x6,%eax                  /* string_length = 6？不是的话引爆 */
  401082:	74 4e                	je     4010d2 <phase_5+0x70>
  401084:	e8 b1 03 00 00       	call   40143a <explode_bomb>
  401089:	eb 47                	jmp    4010d2 <phase_5+0x70>
  40108b:	0f b6 0c 03          	movzbl (%rbx,%rax,1),%ecx         /* ecx = [rbx + rax]的一个字节 */
  40108f:	88 0c 24             	mov    %cl,(%rsp)                 /* [rsp] = cl */
  401092:	48 8b 14 24          	mov    (%rsp),%rdx                /* rdx = cl */
  401096:	83 e2 0f             	and    $0xf,%edx                  /* ascii mod 16 */
  401099:	0f b6 92 b0 24 40 00 	movzbl 0x4024b0(%rdx),%edx        /* edx = [rdx + 0x4024b0] */
  4010a0:	88 54 04 10          	mov    %dl,0x10(%rsp,%rax,1)      /* [rsp + rax + 16] = dl */
  4010a4:	48 83 c0 01          	add    $0x1,%rax                  /* rax + 1 */
  4010a8:	48 83 f8 06          	cmp    $0x6,%rax                  /* rax != 6 的话，
  4010ac:	75 dd                	jne    40108b <phase_5+0x29>      /* 循环，回到 movzbl (%rbx,%rax,1),%ecx */
  4010ae:	c6 44 24 16 00       	movb   $0x0,0x16(%rsp)            /* [rsp + 16] = 0 */
  4010b3:	be 5e 24 40 00       	mov    $0x40245e,%esi             /* esi = 0x40245e */
  4010b8:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi            /* rdi = &[rsp + 16] */
  4010bd:	e8 76 02 00 00       	call   401338 <strings_not_equal>
  4010c2:	85 c0                	test   %eax,%eax                  /* 如果字符串相同，返回*/
  4010c4:	74 13                	je     4010d9 <phase_5+0x77>
  4010c6:	e8 6f 03 00 00       	call   40143a <explode_bomb>
  4010cb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
  4010d0:	eb 07                	jmp    4010d9 <phase_5+0x77>
  4010d2:	b8 00 00 00 00       	mov    $0x0,%eax                  /* eax = 0 跳到 movzbl $0x6,%eax */
  4010d7:	eb b2                	jmp    40108b <phase_5+0x29>      
  4010d9:	48 8b 44 24 18       	mov    0x18(%rsp),%rax
  4010de:	64 48 33 04 25 28 00 	xor    %fs:0x28,%rax
  4010e5:	00 00 
  4010e7:	74 05                	je     4010ee <phase_5+0x8c>
  4010e9:	e8 42 fa ff ff       	call   400b30 <__stack_chk_fail@plt>
  4010ee:	48 83 c4 20          	add    $0x20,%rsp
  4010f2:	5b                   	pop    %rbx
  4010f3:	c3                   	ret
```
在0x4010ae设了一个断点，看到要比较的字符串是“flyers”，显然这个循环会对我们输入的字符串进行处理。
`movzbl (%rbx,%rax,1),%ecx` 看起来是读取ascii。
0x4024b0 处：
`0x4024b0 <array.3449>:	"maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"`
写成数学问题会直观一些，等同与解题：设$\ ascii码 = c_i, (i ∈[1,6])$，$\ c_i\mod 16 ≡ 9, 15, 14, 5, 6, 7$（"flyers"中每个字母对应的偏移），求解$\ c_i$。
手算的话，$\ c_i$一定是在可见字符里面的，故 $\ 31 <= c_i <= 126$。
所以 $\ c_i = 16 * k + r_i\,(i ∈[1,6],r_i分别是9, 15, 14, 5, 6, 7。k是正整数) $
全部的解之组合，就是答案了，所以答案不唯一。
我给出`9oN5FG`。

## Phase 6
~~眼睛疼，有点长;(~~
```
00000000004010f4 <phase_6>:
  4010f4:	41 56                	push   %r14
  4010f6:	41 55                	push   %r13
  4010f8:	41 54                	push   %r12
  4010fa:	55                   	push   %rbp
  4010fb:	53                   	push   %rbx
  4010fc:	48 83 ec 50          	sub    $0x50,%rsp                     /* 80字节的栈空间 */
  401100:	49 89 e5             	mov    %rsp,%r13                      /* r13 = rsp */
  401103:	48 89 e6             	mov    %rsp,%rsi                      /* rsi = rsp */
  401106:	e8 51 03 00 00       	call   40145c <read_six_numbers>      /* 又来读6个数字了 */
  40110b:	49 89 e6             	mov    %rsp,%r14                      /* r14 = rsp */
  40110e:	41 bc 00 00 00 00    	mov    $0x0,%r12d                     /* r12 = 0 */
  401114:	4c 89 ed             	mov    %r13,%rbp                      /* rbp = r13 （栈底=栈顶？）*/
  401117:	41 8b 45 00          	mov    0x0(%r13),%eax                 /* eax = [r13] */
  40111b:	83 e8 01             	sub    $0x1,%eax                      /* eax -= 1 */
  40111e:	83 f8 05             	cmp    $0x5,%eax                      /* eax < 5 ? 否则引爆 */
  401121:	76 05                	jbe    401128 <phase_6+0x34>
  401123:	e8 12 03 00 00       	call   40143a <explode_bomb>
  401128:	41 83 c4 01          	add    $0x1,%r12d                     /* r12 += 1 */
  40112c:	41 83 fc 06          	cmp    $0x6,%r12d                     /* r12 == 6? 处理完6个数字跳转下个循环*/
  401130:	74 21                	je     401153 <phase_6+0x5f>
  401132:	44 89 e3             	mov    %r12d,%ebx                     /* ebx = r12 */
  401135:	48 63 c3             	movslq %ebx,%rax                      /* ebx = rax的低32位+补高32符号位 */
  401138:	8b 04 84             	mov    (%rsp,%rax,4),%eax             /* eax = [rsp + rax * 4] */
  40113b:	39 45 00             	cmp    %eax,0x0(%rbp)                 /* eax != [rbp]? 否则引爆 */ 
  40113e:	75 05                	jne    401145 <phase_6+0x51>
  401140:	e8 f5 02 00 00       	call   40143a <explode_bomb>
  401145:	83 c3 01             	add    $0x1,%ebx                      /* ebx += 1 */
  401148:	83 fb 05             	cmp    $0x5,%ebx                      /* ebx >= 5，循环回 ebx = rax */
  40114b:	7e e8                	jle    401135 <phase_6+0x41>
  40114d:	49 83 c5 04          	add    $0x4,%r13                      /* r13 += 4 */
  401151:	eb c1                	jmp    401114 <phase_6+0x20>          /* 循环 1114 rbp = r13 */
  401153:	48 8d 74 24 18       	lea    0x18(%rsp),%rsi                /* rsi = &[rsp + 24] */
  401158:	4c 89 f0             	mov    %r14,%rax                      /* rax = r14 */
  40115b:	b9 07 00 00 00       	mov    $0x7,%ecx                      /* ecx = 7 */
  401160:	89 ca                	mov    %ecx,%edx                      /* edx = 7 */
  401162:	2b 10                	sub    (%rax),%edx                    /* edx -= [rax] （7 - [每个值]） */
  401164:	89 10                	mov    %edx,(%rax)                    /* [rax] = edx */
  401166:	48 83 c0 04          	add    $0x4,%rax                      /* rax += 4 */
  40116a:	48 39 f0             	cmp    %rsi,%rax                      /* rsi != rax ? 循环回 edx = 7 */
  40116d:	75 f1                	jne    401160 <phase_6+0x6c>
  40116f:	be 00 00 00 00       	mov    $0x0,%esi                      /* esi = 0，到1197 */
  401174:	eb 21                	jmp    401197 <phase_6+0xa3>
  401176:	48 8b 52 08          	mov    0x8(%rdx),%rdx                 /* rdx += [rdx + 8] */
  40117a:	83 c0 01             	add    $0x1,%eax                      /* eax += 1 */
  40117d:	39 c8                	cmp    %ecx,%eax                      /* ecx != eax, 循环回rdx += [rdx + 8]*/
  40117f:	75 f5                	jne    401176 <phase_6+0x82>
  401181:	eb 05                	jmp    401188 <phase_6+0x94>
  401183:	ba d0 32 60 00       	mov    $0x6032d0,%edx                 /* 链表开始的地址 */
  401188:	48 89 54 74 20       	mov    %rdx,0x20(%rsp,%rsi,2)         /* [rsp + rsi * 2 + 20] = rdx */
  40118d:	48 83 c6 04          	add    $0x4,%rsi                      /* rsi += 4 */
  401191:	48 83 fe 18          	cmp    $0x18,%rsi                     /* rsi == 24? 跳去 11ab */
  401195:	74 14                	je     4011ab <phase_6+0xb7>
  401197:	8b 0c 34             	mov    (%rsp,%rsi,1),%ecx              /* ecx = [rsp + rsi] */
  40119a:	83 f9 01             	cmp    $0x1,%ecx                       /* ecx >= 1? 到 1183 */
  40119d:	7e e4                	jle    401183 <phase_6+0x8f>
  40119f:	b8 01 00 00 00       	mov    $0x1,%eax
  4011a4:	ba d0 32 60 00       	mov    $0x6032d0,%edx                  /* 链表开始的地址 */
  4011a9:	eb cb                	jmp    401176 <phase_6+0x82>
  4011ab:	48 8b 5c 24 20       	mov    0x20(%rsp),%rbx                 /* rbx = [rsp + 20] */
  4011b0:	48 8d 44 24 28       	lea    0x28(%rsp),%rax                 /* rax = &[rsp + 40] */
  4011b5:	48 8d 74 24 50       	lea    0x50(%rsp),%rsi                 /* rsi = [rsp + 80] */
  4011ba:	48 89 d9             	mov    %rbx,%rcx                       /* rcx = rbx */
  4011bd:	48 8b 10             	mov    (%rax),%rdx                     /* rdx = [rax] */
  4011c0:	48 89 51 08          	mov    %rdx,0x8(%rcx)                  /* [rcx + 8] = rdx */
  4011c4:	48 83 c0 08          	add    $0x8,%rax                       /* rax += 8 */
  4011c8:	48 39 f0             	cmp    %rsi,%rax                       /* rax == rsi? 跳到下个循环11d2 */
  4011cb:	74 05                	je     4011d2 <phase_6+0xde>
  4011cd:	48 89 d1             	mov    %rdx,%rcx                       /* rdx = rcx, 循环rdx = [rax] */
  4011d0:	eb eb                	jmp    4011bd <phase_6+0xc9>
  4011d2:	48 c7 42 08 00 00 00 	movq   $0x0,0x8(%rdx)                  /* [rdx + 8] = 0 */
  4011d9:	00 
  4011da:	bd 05 00 00 00       	mov    $0x5,%ebp                       /* ebp = 5 */
  4011df:	48 8b 43 08          	mov    0x8(%rbx),%rax                  /* rax = [rbx + 8] */
  4011e3:	8b 00                	mov    (%rax),%eax                     /* eax = [rax] */
  4011e5:	39 03                	cmp    %eax,(%rbx)                     /* [rbx] >= eax? 不是就引爆 */
  4011e7:	7d 05                	jge    4011ee <phase_6+0xfa>
  4011e9:	e8 4c 02 00 00       	call   40143a <explode_bomb>
  4011ee:	48 8b 5b 08          	mov    0x8(%rbx),%rbx                  /* rbx = [rbx + 8] */
  4011f2:	83 ed 01             	sub    $0x1,%ebp                       /* ebp -= 1 */
  4011f5:	75 e8                	jne    4011df <phase_6+0xeb>           /* ebp != 0, 循环 rax = [rbx + 8] */
  4011f7:	48 83 c4 50          	add    $0x50,%rsp                      /* 返回之前的准备... */
  4011fb:	5b                   	pop    %rbx
  4011fc:	5d                   	pop    %rbp
  4011fd:	41 5c                	pop    %r12
  4011ff:	41 5d                	pop    %r13
  401201:	41 5e                	pop    %r14
  401203:	c3                   	ret
```
这题，一共有五个循环，还是要读取六个数字。但是这次栈分配的空间变大了，为什么呢？
第一个循环要求要6个相异数字，检查 $ eax - 1 < 5 $，用jbe做无符号比较，就是要求输入值在[1,6]之间。
[参考](https://earthaa.github.io/2020/01/12/CSAPP-Bomblab/)了一下，得知用到了链表，所以栈空间才变大了。
第二个循环是用数字7来减去每个储存的值，并储存回 [rax] 的相应位置，处理完了就到下一个循环。
第三个循环，`mov $0x6032d0,%edx` 看得出 0x6032d0 是链表开始的地方。每次 [rdx + 8] 就知道是访问 node.next ，next 一般在内存地址读取偏移 + 8 的地方，但是从一个 node 到下一个，要16字节。得到
```
0x6032d0   c1   → 0x6032e0
0x6032e0   c2   → 0x6032f0
0x6032f0   c3   → 0x603300
0x603300   c4   → 0x603310
0x603310   c5   → 0x603320
0x603320   c6   → NULL
```
第五个循环就是检查倒叙排列，`rbx = [rbx + 8]，[rbx] >= eax`。
第四个循环好像是重排链表，我看了半天没明白逻辑，所以gdb调试看看。
break *0x4011ab
随便输入`4 3 2 5 6 1`
```
(gdb) x/wx 0x6032d0
0x6032d0 <node1>:	0x0000014c
(gdb) x/wx 0x6032d0+16
0x6032e0 <node2>:	0x000000a8
(gdb) x/wx 0x6032d0+32
0x6032f0 <node3>:	0x0000039c
(gdb) x/wx 0x6032d0+48
0x603300 <node4>:	0x000002b3
(gdb) x/wx 0x6032d0+64
0x603310 <node5>:	0x000001dd
(gdb) x/wx 0x6032d0+80
0x603320 <node6>:	0x000001bb
```
我跑很多次循环都没有变值。
 
换个输入`1 2 3 4 5 6`
依旧是：
```
(gdb) x/wx 0x6032d0
0x6032d0 <node1>:	0x0000014c
(gdb) x/wx 0x6032d0+16
0x6032e0 <node2>:	0x000000a8
(gdb) x/wx 0x6032d0+32
0x6032f0 <node3>:	0x0000039c
(gdb) x/wx 0x6032d0+48
0x603300 <node4>:	0x000002b3
(gdb) x/wx 0x6032d0+64
0x603310 <node5>:	0x000001dd
(gdb) x/wx 0x6032d0+80
0x603320 <node6>:	0x000001bb
```
嗯？和输入没有关系（？）。我感觉题目的意图应该是排序这六个寄存器的值，从大到小`3 4 5 6 1 2`。
但是这些输入要和7做差，所以输入了`4 3 2 1 6 5`。


