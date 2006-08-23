package jep;


import java.util.Vector;
import java.io.FileReader;

import org.apache.bsf.*;
import org.apache.bsf.util.*;


/**
 * <pre>
 * BSFJepEngine.java - Apache BSF support.
 *
 * Copyright (c) 2004, 2005 Mike Johnson.
 *
 * This file is licenced under the the zlib/libpng License.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 * 
 *     1. The origin of this software must not be misrepresented; you
 *     must not claim that you wrote the original software. If you use
 *     this software in a product, an acknowledgment in the product
 *     documentation would be appreciated but is not required.
 * 
 *     2. Altered source versions must be plainly marked as such, and
 *     must not be misrepresented as being the original software.
 * 
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 *
 * Created: Sat Jun 12 19:41:04 2004
 * </pre>
 *
 * @author [mrjohnson0 at sourceforge.net] Mike Johnson
 * @version $Id$
 */
public class BSFJepEngine extends BSFEngineImpl {

    private Jep jep = null;


    /**
     * for testing.
     *
     * @param args a <code>String[]</code> value
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
    
    
    /**
     * Describe <code>propertyChange</code> method here.
     *
     * @param e a <code>java.beans.PropertyChangeEvent</code> value
     */
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
            jep.set(bean.name, (String) null);
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
