// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2021 FAU -- Joachim Falk <joachim.falk@fau.de>
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
package de.fau.scd.VPC.io;

import java.util.Iterator;

public class Common {

    /**
     * Gets an iterable list of child elements named {@param childName} of the
     * parent element {@param parentElement}.
     *
     * @param parentElement
     *            the parent element
     * @param childName
     *            the tag name of the desired child elements
     * @return the iterable element objects
     */
    public static Iterable<org.w3c.dom.Element> childElements(final org.w3c.dom.Element parentElement, final String childName) {
        return new Iterable<org.w3c.dom.Element>() {

            @Override
            public Iterator<org.w3c.dom.Element> iterator() {
                return new Iterator<org.w3c.dom.Element>() {
                    private org.w3c.dom.Element cur = null;
                    private int c = -1;
                    private final org.w3c.dom.NodeList nodes = parentElement.getChildNodes();

                    {
                        skip();
                    }

                    private int skip() {
                        int old = c++;
                        while (hasNext() && !nodes.item(c).getNodeName().equals(childName))
                            ++c;
                        return old;
                    }

                    @Override
                    public boolean hasNext() {
                        return nodes.getLength() > c;
                    }

                    @Override
                    public org.w3c.dom.Element next() {
                        cur = (org.w3c.dom.Element) nodes.item(skip());
                        return cur;
                    }

                    @Override
                    public void remove() {
                        if (cur == null)
                            throw new IllegalStateException();
                        parentElement.removeChild(cur);
                        --c;
                    }
                };
            }
        };
    }

    /**
     * Gets the single child element named {@param childName} of the parent
     * element {@param parentElement}. If there are more than one or no child
     * elements with the requested name, an exception is thrown.
     *
     * @param parentElement
     *            the parent element
     * @param childName
     *            the tag name of the desired child elements
     * @return the desired child element
     * @throws SNGFormatErrorException
     */
    public static org.w3c.dom.Element childElement(final org.w3c.dom.Element parentElement, final String childName)
            throws FormatErrorException {
        return childElement(parentElement, childName, false);
    }

    /**
     * Gets the single child element named {@param childName} of the parent
     * element {@param parentElement}. If there are more than one child element
     * with the requested name, an exception is thrown. If the element is
     * optional {@param optional} and not present, then null is returned.
     * Otherwise, if not optional and missing an exception is thrown.
     *
     * @param parentElement
     *            the parent element
     * @param childName
     *            the tag name of the desired child elements
     * @param optional
     *            If true, the element is allowed to be missing
     * @return the desired child element or null if optional and the element is
     *         missing
     * @throws SNGFormatErrorException
     */
    public static org.w3c.dom.Element childElement(final org.w3c.dom.Element parentElement, final String childName,
            boolean optional) throws FormatErrorException {
        Iterator<org.w3c.dom.Element> iter = childElements(parentElement, childName).iterator();
        if (!iter.hasNext()) {
            if (!optional)
                throw new FormatErrorException(
                        "Parent element " + parentElement + " is missing a " + childName + " child element!");
            else
                return null;
        }
        org.w3c.dom.Element retval = iter.next();
        if (iter.hasNext())
            throw new FormatErrorException(
                    "Parent element " + parentElement + " must only have one " + childName + " child element!");
        return retval;
    }

    static public class FormatErrorException extends Exception {
        private static final long serialVersionUID = 3741956271022484454L;

        public FormatErrorException(String message) {
            super(message);
        }
    }
}
