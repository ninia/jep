package jep;

/**
 * BSFJepEngine.java
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Created: Sat Jun 12 19:41:04 2004
 *
 * @author <a href="mailto:mrjohnson0@users.sourceforge.net">Mike Johnson</a>
 * @version 1.0
 */

import java.util.Vector;
import java.io.FileReader;

import org.apache.bsf.*;
import org.apache.bsf.util.*;


/**
 * Apache BSF support.
 *
 * @author <a href="mailto:mrjohnson@pbook.local">Mike Johnson</a>
 * @version 1.0
 */
public class BSFJepEngine extends BSFEngineImpl {

    private Jep jep = null;


    /**
     * for testing.
     *
     * @param args[] a <code>String</code> value
     * @exception Exception if an error occurs
     */
    public static void main(String args[]) throws Exception {
        BSFManager.registerScriptingEngine(
            "jep", "jep.BSFJepEngine", new String[] {"jep", "py"});
        BSFManager manager = new BSFManager();
        String filename = "testbsf.py";
        manager.exec("jep",
                     filename,
                     0,
                     0,
                     filename);
        System.out.println("a = " + manager.eval("jep", "this", 0, 0, "a"));
    }
    
    
    /**
     * Initialize the engine.
     *
     * @param mgr a <code>BSFManager</code> value
     * @param lang a <code>String</code> value
     * @param declaredBeans a <code>Vector</code> value
     * @exception BSFException if an error occurs
     */
    public void initialize(BSFManager mgr,
                           String lang,
                           Vector declaredBeans) throws BSFException {
        super.initialize (mgr, lang, declaredBeans);
        
        try {
            jep = new Jep();
            jep.setClassLoader(mgr.getClassLoader());
            
            // register the mgr with object name "bsf"
            jep.set("bsf", new BSFFunctions(mgr, this));
            
            int size = declaredBeans.size();
            for(int i = 0; i < size; i++)
                declareBean((BSFDeclaredBean) declaredBeans.elementAt(i));
        }
        catch(JepException e) {
            throw new BSFException(BSFException.REASON_EXECUTION_ERROR,
                                   e.toString(), e);
        }
    }
    
    
    /**
     * Describe <code>terminate</code> method here.
     *
     */
    public void terminate() {
        super.terminate();
        
        if(jep != null)
            jep.close();
    }
    
    
    public void propertyChange(java.beans.PropertyChangeEvent e) {
        super.propertyChange(e);
    }


    /**
     * Declare a bean
     *
     * @param bean a <code>BSFDeclaredBean</code> value
     * @exception BSFException if an error occurs
     */
    public void declareBean(BSFDeclaredBean bean) throws BSFException {
        try {
            jep.set(bean.name, bean.bean);
        }
        catch(JepException e) {
            throw new BSFException(BSFException.REASON_EXECUTION_ERROR,
                                   e.toString(), e);
        }
    }
    

    /**
     * Undeclare a bean
     *
     * @param bean a <code>BSFDeclaredBean</code> value
     * @exception BSFException if an error occurs
     */
    public void undeclareBean(BSFDeclaredBean bean) throws BSFException {
        try {
            jep.set(bean.name, null);
        }
        catch(JepException e) {
            throw new BSFException(BSFException.REASON_EXECUTION_ERROR,
                                   e.toString(), e);
        }
    }
    
    
    /**
     * Describe <code>call</code> method here.
     *
     * @param object an <code>Object</code> value
     * @param method a <code>String</code> value
     * @param args an <code>Object[]</code> value
     * @return an <code>Object</code> value
     * @exception BSFException if an error occurs
     */
    public Object call(Object object,
                       String method,
                       Object[] args) throws BSFException {
        throw new BSFException(BSFException.REASON_UNSUPPORTED_FEATURE,
                               "Not implemented.");
    }
    
    
    /**
     * Evaluate an expression.
     *
     * @param source a <code>String</code> value
     * @param lineNo an <code>int</code> value
     * @param columnNo an <code>int</code> value
     * @param script an <code>Object</code> value
     * @return an <code>Object</code> value
     * @exception BSFException if an error occurs
     */
    public Object eval(String source,
                       int lineNo,
                       int columnNo, 
                       Object script) throws BSFException {
        try {
            return jep.getValue(script.toString());
        }
        catch(JepException e) {
            throw new BSFException(BSFException.REASON_EXECUTION_ERROR,
                                   e.toString(), e);
        }
    }
    
    
    /**
     * Execute a script.
     *
     * @param source a <code>String</code> value
     * @param lineNo an <code>int</code> value
     * @param columnNo an <code>int</code> value
     * @param script an <code>Object</code> value
     * @exception BSFException if an error occurs
     */
    public void exec(String source,
                     int lineNo,
                     int columnNo,
                     Object script) throws BSFException {
        try {
            jep.runScript(script.toString());
        }
        catch(JepException e) {
            throw new BSFException(BSFException.REASON_EXECUTION_ERROR,
                                   e.toString(), e);
        }
    }

} // BSFJepEngine
