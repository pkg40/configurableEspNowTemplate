/*
 * MIT License
 *
 * Copyright (c) 2025 Peter K Green (pkg40)
 * Email: pkg40@yahoo.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once
#include <Arduino.h>

const String embeddedStyle = R"rawliteral(
<style>
    body {
        font-family: Arial, sans-serif;
        background-color: #f5f5f5;
        color: #333;
        margin: 20px;
        text-align: center;
    }
    h1, h2 {
        color: #4CAF50;
    }
    table {
        width: 80%;
        margin: auto;
        border-collapse: collapse;
        background: #fff;
    }
    th, td {
        padding: 10px;
        border: 1px solid #ddd;
        text-align: left;
    }
    th {
        background: #4CAF50;
        color: white;
    }
    input[type="text"], input[type="file"] {
        padding: 6px;
        border-radius: 4px;
        border: 1px solid #ccc;
    }
    input[type="submit"], button {
        background: #4CAF50;
        color: white;
        padding: 10px 15px;
        border: none;
        cursor: pointer;
    }
    a {
        color: #4CAF50;
        text-decoration: none;
        font-weight: bold;
    }
    a:hover {
        text-decoration: underline;
    }
</style>
)rawliteral";