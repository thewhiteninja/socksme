# socksme
<div style="width:90%">  
  <a href="https://nodejs.org/api/documentation.html#documentation_stability_index">
    <img src="https://img.shields.io/badge/stability-experimental-orange.svg?style=flat-square" alt="API stability" />
  </a> &nbsp; 
  <a href="https://img.shields.io/badge">
    <img src="https://img.shields.io/badge/Platform-Win32-00aed5.svg" alt="Win32" />
  </a> &nbsp;
  <a href="https://img.shields.io/badge">
    <img src="https://img.shields.io/badge/Platform-Win64-00aed5.svg" alt="Win64" />
  </a> &nbsp;
  <a href="https://img.shields.io/badge">
    <img src="https://img.shields.io/badge/Language-C%2B%2B-brightgreen.svg" alt="C++" />
  </a> &nbsp;
  <a href="https://img.shields.io/badge">
    <img src="https://img.shields.io/badge/Compiler-Visual%20Studio%202017-865fc5.svg" alt="Visual Studio 2017" />
  </a>  
</div>

## Description

PoC in C++ of a Socks5 proxy.

Default creds: root/toor


## RFC

- [RFC 1928 - SOCKS Protocol Version 5](https://tools.ietf.org/html/rfc1928)
- [RFC 1929 - Username/Password Authentication for SOCKS V5](https://tools.ietf.org/html/rfc1929)
- [RFC 1961 - GSS-API Authentication Method for SOCKS Version 5](https://tools.ietf.org/html/rfc1961)


## Usage

```sh
λ socksme.exe -h
Usage : socksme.exe [options]

    Options :
        -h, --help : show this message

        -s, --source-addr : set local ip address    (0.0.0.0)
        -p, --source-port : set local port          (8888)
        -b, --backlog     : set backlog value       (5)

        -v, --log-level   : set logging level [0-2] (1)

```

## Example

```sh
λ socksme.exe
Starting socksme.exe at Thu Nov 29 20:57:29 2018 (version 0.9.0.0)

[29 Nov 2018 20:57:29] - INFO - Socks server running on 0.0.0.0:8888
[29 Nov 2018 20:58:27] - INFO - CONNECT to www.google.com:80
[29 Nov 2018 20:58:48] - INFO - CONNECT to www.facebook.com:80
[29 Nov 2018 20:59:16] - INFO - CONNECT to www.github.com:443
[29 Nov 2018 20:59:20] - INFO - Socks server stopped
```
