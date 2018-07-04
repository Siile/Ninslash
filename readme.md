<div align="center">
<img alt="Ninslash" src="http://ninslash.com/png/logo.png" />
<br>
<br>
<br>
<a href="http://ninslash.com/download/"><strong>&gt;&gt; Download &lt;&lt;</strong></a>
<br>
<br>
<img alt="Ninslash" src="http://ninslash.com/png/0.4.4.png" />
</div>
<br>
<br>
<strong>Compiling in windows</strong>
<br>
<strong>Required tools</strong>
<a href="http://ninslash.com/tools/bam-0.4.0.zip">Bam 0.4.0</a>
<a href="https://www.python.org/downloads/">Python (tested with 3.7.0)</a>
<a href="https://visualstudio.microsoft.com/vs/older-downloads/">Visual Studio Express 2010</a>
<a href="https://visualstudio.microsoft.com/vs/older-downloads/">Visual Studio 2010 Service Pack 1</a>
<br>
<br>
1. Install everything
2. Compile Bam (run make_win32_msvc.bat)
3. Open console (cmd) in Ninslash root folder
4. Define Visual Studio environment (%comspec% /k ""C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"" x86)
5. Run Bam (c:\bam\bam release)
<br>
<img alt="Ninslash" src="http://ninslash.com/png/compile.png" />
<br>
<br>
<br>
If you're using unix, having a look at <a href="https://github.com/Siile/Ninslash/blob/master/circle.yml">circleci instructions</a> might be helpful