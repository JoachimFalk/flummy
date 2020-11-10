// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */
package de.fau.scd.SNG;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import net.sf.opendse.model.Attributes;
import net.sf.opendse.model.Communication;
import net.sf.opendse.model.Dependency;
import net.sf.opendse.model.IAttributes;
import net.sf.opendse.model.Link;
import net.sf.opendse.model.Mapping;
import net.sf.opendse.model.Resource;
import net.sf.opendse.model.Task;
import net.sf.opendse.model.parameter.ParameterRange;
import net.sf.opendse.model.parameter.ParameterRangeDiscrete;
import net.sf.opendse.model.parameter.ParameterSelect;
import net.sf.opendse.model.parameter.ParameterUniqueID;

import org.apache.commons.collections15.BidiMap;
import org.apache.commons.collections15.bidimap.DualHashBidiMap;

/**
 * The {@code Common} class contains common methods for reading and writing a
 * {@code Specification}.
 * 
 * @author Martin Lukasiewycz
 * 
 */
public class Common {
    
    public static final String NS = "http://cs12.fau.de/sgx/v1";

    public static BidiMap<String, Class<?>> classMap = new DualHashBidiMap<String, Class<?>>();

    static {
        classMap.put("INT", Integer.class);
        classMap.put("DOUBLE", Double.class);
        classMap.put("STRING", String.class);
        classMap.put("BOOL", Boolean.class);
        classMap.put("RANGE", ParameterRange.class);
        classMap.put("DISCRETERANGE", ParameterRangeDiscrete.class);
        classMap.put("SELECT", ParameterSelect.class);
        classMap.put("UID", ParameterUniqueID.class);
        classMap.put("resource", Resource.class);
        classMap.put("link", Link.class);
        classMap.put("task", Task.class);
        classMap.put("communication", Communication.class);
        classMap.put("dependency", Dependency.class);
        classMap.put("mapping", Mapping.class);
        classMap.put("MAP", HashMap.class);
        classMap.put("SET", HashSet.class);
        classMap.put("LIST", ArrayList.class);
    }

    protected static Set<Class<?>> primitives = new HashSet<Class<?>>();

    static {
        primitives.add(Boolean.class);
        primitives.add(Byte.class);
        primitives.add(Integer.class);
        primitives.add(Character.class);
        primitives.add(Short.class);
        primitives.add(Float.class);
        primitives.add(Long.class);
        primitives.add(Double.class);
    }

    protected static boolean isPrimitive(Class<?> cls) {
        return cls.isPrimitive() || primitives.contains(cls);
    }

    protected static String getType(Class<?> clazz) {
        if (classMap.containsValue(clazz)) {
            return classMap.getKey(clazz);
        } else {
            return clazz.getCanonicalName().toString();
        }
    }

    @SuppressWarnings("rawtypes")
    protected static Object toInstance(String value, Class<?> clazz) throws IllegalArgumentException, SecurityException,
            InstantiationException, IllegalAccessException, InvocationTargetException, NoSuchMethodException {
        if (!clazz.isEnum()) {
            Constructor constructor = clazz.getConstructor(String.class);
            if (constructor != null) {
                return constructor.newInstance(value.trim());
            }
        } else {
            Class<? extends Enum> eclazz = clazz.asSubclass(Enum.class);
            for (Enum e : eclazz.getEnumConstants()) {
                if (e.name().equalsIgnoreCase(value.trim())) {
                    return e;
                }
            }
        }
        return null;
    }

    protected static void setAttributes(IAttributes e, Attributes attributes) {
        for (String name : attributes.keySet()) {
            e.setAttribute(name, attributes.get(name));
        }
    }

    /**
     * Transforms a Base64 string into an object.
     * 
     * @param s
     *            the string
     * @return the object
     * @throws IOException
     *             thrown in case of an IO error
     * @throws ClassNotFoundException
     *             thrown in case the class does not exist
     */
    public static Object fromString(String s) throws IOException, ClassNotFoundException {
        byte[] data = Base64Coder.decode(s);
        ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(data));
        Object o = ois.readObject();
        ois.close();
        return o;
    }

    /**
     * Transforms an object into a Base64 string.
     * 
     * @param o
     *            the object
     * @return the string
     * @throws IOException
     *             thrown in case of an IO error
     */
    public static String toString(Serializable o) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        ObjectOutputStream oos = new ObjectOutputStream(baos);
        oos.writeObject(o);
        oos.close();

        return new String(Base64Coder.encode(baos.toByteArray()));

    }

}
