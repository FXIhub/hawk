#!/usr/bin/perl -w

# This is the script that parses the latex documentation embedded in 
# comments of the the source files and output the requires png files.

# Here's an example of documentation taken from metadata.c:
#  HIO formula 
# ##LaTeX code begin for file HawkGUI/images/hio_formula.png##

# $\rho^{(n+1)}(r) = \begin{cases} 
# {\textbf{\em P}}_m \rho^{(n)}(r)  & \mbox{if } r \in S \\ 
# ({\textbf{\em I}} - \beta {\textbf{\em P}}_m) \rho^{(n)}(r)  & \mbox{if }  r  \notin  S 
# \end{cases}
# $
# \begin{itemize}
# \item $\rho^{(n)}$ is the real space image after $n$ iterations.
# \item $S$ is the support
# \item ${\textbf{\em I}}$ is the identity operator
# \item ${\textbf{\em P}}_m$ is the modulus projection operator
# \item $\beta$ is a relaxation parameter
# \end{itemize}
# ##LaTeX code end##

use File::Temp;
use FileHandle;
use File::Basename;

$preamble = <<END;
\\documentclass[8pt]{article}
\\usepackage[usenames]{color} %used for font color
\\usepackage{amssymb} %maths
\\usepackage{amsmath} %maths
\\usepackage{mathabx} %maths
\\usepackage[utf8]{inputenc} %useful to type directly diacritic characters
\\pagestyle{empty}
\\begin{document}
\\begin{center}
END

$postscript = <<END;
\\end{center}
\\end{document}
END

my($foo, $base_dir, $suffix) = fileparse($ARGV[0]);
$latex_region = 0;
while(<>){
  if(/##LaTeX code end##/){
    print $fh $postscript;
    $fh->close();
    print $filename."\n";
    my($file, $dir, $suffix) = fileparse($filename,".tex");
    system("cd $dir && latex $filename");
    system("cd $dir && dvips -E $file.dvi");
    system("convert -units PixelsPerInch -density 110 $dir/$file.ps $base_dir/$output");
    $latex_region = 0;
  }  
  if($latex_region){
    print $fh $_;
  }
  if(/##LaTeX code begin for file (.*)##/){
     $latex_region = 1;
     # the output path is relative to the location of the input 
     $output = $1;
    ($fh, $filename) = File::Temp::tempfile(SUFFIX => '.tex');
    print $fh $preamble;
  }  
}
system("touch $base_dir/HawkGUI/hawk.qrc")
