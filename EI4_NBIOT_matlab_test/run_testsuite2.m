import matlab.unittest.plugins.CodeCoveragePlugin
import matlab.unittest.plugins.codecoverage.CoverageReport
runner = testrunner("textoutput");
sourceCodeFolder = "src";
reportFolder = "coverageReport";
reportFormat = CoverageReport(reportFolder);
p = CodeCoveragePlugin.forFolder('src',"Producing",reportFormat);
runner.addPlugin(p);
 
suite = testsuite("TestSuite2");
results = runner.run(suite);
result = runtests('TestSuite2')
rt = table(result)